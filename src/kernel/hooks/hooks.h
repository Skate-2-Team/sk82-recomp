#pragma once

#include <windows.h>
#include <iostream>
#include <profileapi.h>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <vector>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/heap.h"
#include "kernel/function.h"
#include "kernel/xdm.h"
#include "cpu/guest_thread.h"

#include "kernel/xbox.h"
#include "xex.h"
#include <unordered_set>

namespace Hooks
{
    // structs
    // events
    enum NTSTATUS
    {
        NTSTATUS_SUCCESS = 0,
        NTSTATUS_TIMEOUT = 1,
        NTSTATUS_UNSUCCESSFUL = -1
    };

    struct Event final : KernelObject, HostObject<XKEVENT>
    {
        bool manualReset;
        std::atomic<bool> signaled;

        Event(XKEVENT *header)
            : manualReset(!header->Type), signaled(!!header->SignalState)
        {
        }

        Event(bool manualReset, bool initialState)
            : manualReset(manualReset), signaled(initialState)
        {
        }

        uint32_t Wait(uint32_t timeout) override
        {
            if (timeout == 0)
            {
                if (manualReset)
                {
                    if (!signaled)
                        return STATUS_TIMEOUT;
                }
                else
                {
                    bool expected = true;
                    if (!signaled.compare_exchange_strong(expected, false))
                        return STATUS_TIMEOUT;
                }
            }
            else if (timeout == INFINITE)
            {
                if (manualReset)
                {
                    signaled.wait(false);
                }
                else
                {
                    while (true)
                    {
                        bool expected = true;
                        if (signaled.compare_exchange_weak(expected, false))
                            break;

                        signaled.wait(expected);
                    }
                }
            }
            else
            {
                assert(false && "Unhandled timeout value.");
            }

            return NTSTATUS_SUCCESS;
        }

        bool Set()
        {
            signaled = true;

            if (manualReset)
                signaled.notify_all();
            else
                signaled.notify_one();

            return TRUE;
        }

        bool Reset()
        {
            signaled = false;
            return TRUE;
        }
    };

    struct Semaphore final : KernelObject, HostObject<XKSEMAPHORE>
    {
        std::atomic<uint32_t> count;
        uint32_t maximumCount;

        Semaphore(XKSEMAPHORE *semaphore)
            : count(semaphore->Header.SignalState), maximumCount(semaphore->Limit)
        {
        }

        Semaphore(uint32_t count, uint32_t maximumCount)
            : count(count), maximumCount(maximumCount)
        {
        }

        uint32_t Wait(uint32_t timeout) override
        {
            if (timeout == 0)
            {
                uint32_t currentCount = count.load();
                if (currentCount != 0)
                {
                    if (count.compare_exchange_weak(currentCount, currentCount - 1))
                        return NTSTATUS_SUCCESS;
                }

                return STATUS_TIMEOUT;
            }
            else if (timeout == INFINITE)
            {
                uint32_t currentCount;
                while (true)
                {
                    currentCount = count.load();
                    if (currentCount != 0)
                    {
                        if (count.compare_exchange_weak(currentCount, currentCount - 1))
                            return NTSTATUS_SUCCESS;
                    }
                    else
                    {
                        count.wait(0);
                    }
                }

                return NTSTATUS_SUCCESS;
            }
            else
            {
                assert(false && "Unhandled timeout value.");
                return STATUS_TIMEOUT;
            }
        }

        void Release(uint32_t releaseCount, uint32_t *previousCount)
        {
            if (previousCount != nullptr)
                *previousCount = count;

            assert(count + releaseCount <= maximumCount);

            count += releaseCount;
            count.notify_all();
        }
    };

    struct Mutant final : KernelObject
    {
        std::mutex mtx;
        std::condition_variable cv;

        bool signaled;
        uint32_t ownerThreadId;
        uint32_t recursionCount;

        Mutant(bool initialOwner)
            : signaled(!initialOwner),
              ownerThreadId(initialOwner ? GetCurrentThreadId() : 0),
              recursionCount(initialOwner ? 1 : 0)
        {
        }

        uint32_t Wait(uint32_t timeout) override
        {
            uint32_t currentThread = GetCurrentThreadId();
            std::unique_lock<std::mutex> lock(mtx);

            if (ownerThreadId == currentThread)
            {
                recursionCount++;
                return NTSTATUS_SUCCESS;
            }

            if (timeout == 0)
            {
                if (!signaled)
                    return STATUS_TIMEOUT;

                signaled = false;
                ownerThreadId = currentThread;
                recursionCount = 1;
                return NTSTATUS_SUCCESS;
            }
            else if (timeout == INFINITE)
            {
                cv.wait(lock, [this]()
                        { return signaled; });

                signaled = false;
                ownerThreadId = currentThread;
                recursionCount = 1;
                return NTSTATUS_SUCCESS;
            }
            else
            {
                assert(false && "Unhandled timeout value.");
                return STATUS_TIMEOUT;
            }
        }

        bool Release()
        {
            std::lock_guard<std::mutex> lock(mtx);
            uint32_t currentThread = GetCurrentThreadId();

            if (ownerThreadId != currentThread)
            {
                Log::Error("Mutant::Release", "Thread ", currentThread, " attempted to release a mutant owned by thread ", ownerThreadId);
                return false;
            }

            if (recursionCount > 1)
            {
                recursionCount--;
            }
            else
            {
                recursionCount = 0;
                ownerThreadId = 0;
                signaled = true;
                cv.notify_one();
            }

            return true;
        }
    };

    struct Timer final : KernelObject
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool signaled;
        bool active;
        uint32_t period;
        std::chrono::steady_clock::time_point dueTime;

        Timer(bool autoReset) : signaled(false), active(false), period(0) {}

        uint32_t Wait(uint32_t timeout) override
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (timeout == 0)
            {
                if (!signaled)
                    return STATUS_TIMEOUT;

                signaled = false;
                return NTSTATUS_SUCCESS;
            }
            else if (timeout == INFINITE)
            {
                cv.wait(lock, [this]()
                        { return signaled; });

                signaled = false;
                return NTSTATUS_SUCCESS;
            }
            else
            {
                assert(false && "Unsupported timeout value");
                return NTSTATUS_TIMEOUT;
            }
        }

        void SetTimer(uint32_t dueTimeMs, uint32_t periodMs)
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                active = true;
                period = periodMs;
                if (dueTimeMs == 0)
                {
                    signaled = true;
                    cv.notify_all();
                }
                else
                {
                    signaled = false;
                    dueTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(dueTimeMs);
                }
            }

            if (dueTimeMs > 0 || periodMs > 0)
            {
                std::thread([this, dueTimeMs, periodMs]
                            {
                    while (true)
                    {
                        std::unique_lock<std::mutex> lock(mtx);

                        if (!active)
                            break;

                        if (dueTimeMs > 0)
                        {
                            cv.wait_until(lock, dueTime);
                            if (!active)
                                break;
                        }

                        signaled = true;
                        cv.notify_all();

                        if (periodMs == 0)
                        {
                            active = false;
                            break;
                        }

                        dueTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(periodMs);
                        signaled = false;
                    } })
                    .detach();
            }
        }

        void CancelTimer()
        {
            std::lock_guard<std::mutex> lock(mtx);
            active = false;
            signaled = false;
            cv.notify_all();
        }

        bool IsSignaled()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return signaled;
        }
    };

    struct FileHandle : KernelObject
    {
        std::fstream stream;
        std::filesystem::path path;
    };

    struct XamListener : KernelObject
    {
        uint32_t id{};
        uint64_t areas{};
        std::vector<std::tuple<uint32_t, uint32_t>> notifications;

        XamListener(const XamListener &) = delete;
        XamListener &operator=(const XamListener &) = delete;

        XamListener();
        ~XamListener();
    };

    inline uint32_t GuestTimeoutToMilliseconds(be<int64_t> *timeout)
    {
        return timeout ? (*timeout * -1) / 10000 : INFINITE;
    }

    // global vars
    // threading
    inline std::vector<size_t> g_tlsFreeIndices;
    inline size_t g_tlsNextIndex = 0;
    inline Mutex g_tlsAllocationMutex;

    // filesystem
    inline std::vector<HANDLE> g_openedHandles{};
}