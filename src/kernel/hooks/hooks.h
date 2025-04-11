#pragma once

#include <windows.h>
#include <iostream>
#include <profileapi.h>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <vector>
#include <ankerl/unordered_dense.h>
#include <map>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/heap.h"
#include "kernel/function.h"
#include "kernel/xdm.h"
#include "cpu/guest_thread.h"

#include "kernel/xbox.h"
#include "xex.h"
#include <unordered_set>

#ifdef _WIN32
#include <ntstatus.h>
#endif

namespace Hooks
{
    // structs
    // events

    inline std::string ResolvePath(std::string fileName)
    {
        fileName.erase(fileName.begin(), fileName.begin() + 3);

        std::string fullPath = "game\\" + fileName;

        return fullPath;
    }

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
                auto start = std::chrono::steady_clock::now();
                auto end = start + std::chrono::milliseconds(timeout);

                if (manualReset)
                {
                    while (!signaled)
                    {
                        if (std::chrono::steady_clock::now() >= end)
                            return STATUS_TIMEOUT;

                        signaled.wait(false);
                    }
                }
                else
                {
                    while (true)
                    {
                        bool expected = true;
                        if (signaled.compare_exchange_weak(expected, false))
                            break;

                        if (std::chrono::steady_clock::now() >= end)
                            return STATUS_TIMEOUT;

                        signaled.wait(expected);
                    }
                }
            }

            return STATUS_SUCCESS;
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

    // Might need tweaking
    struct Semaphore final : KernelObject, HostObject<XKSEMAPHORE>
    {
        std::atomic<uint32_t> count;
        uint32_t maximumCount;
        std::mutex mutex;
        std::condition_variable cv;

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
                        return STATUS_SUCCESS;
                }

                return STATUS_TIMEOUT;
            }
            else if (timeout == INFINITE)
            {
                std::unique_lock<std::mutex> lock(mutex);

                cv.wait(lock, [this]()
                        { return count.load() > 0; });

                count.fetch_sub(1);
                return STATUS_SUCCESS;
            }
            else
            {
                std::unique_lock<std::mutex> lock(mutex);

                if (cv.wait_for(lock, std::chrono::milliseconds(timeout),
                                [this]()
                                { return count.load() > 0; }))

                {
                    count.fetch_sub(1);
                    return STATUS_SUCCESS;
                }

                return STATUS_TIMEOUT;
            }
        }

        void Release(uint32_t releaseCount, uint32_t *previousCount)
        {
            if (previousCount != nullptr)
                *previousCount = count;

            assert(count + releaseCount <= maximumCount);

            {
                std::lock_guard<std::mutex> lock(mutex);
                count += releaseCount;
            }

            if (releaseCount == 1)
                cv.notify_one();
            else
                cv.notify_all();
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
                return STATUS_SUCCESS;
            }

            if (timeout == 0)
            {
                if (!signaled)
                    return STATUS_TIMEOUT;

                signaled = false;
                ownerThreadId = currentThread;
                recursionCount = 1;
                return STATUS_SUCCESS;
            }
            else if (timeout == INFINITE)
            {
                cv.wait(lock, [this]()
                        { return signaled; });

                signaled = false;
                ownerThreadId = currentThread;
                recursionCount = 1;
                return STATUS_SUCCESS;
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
                // Log::Error("Mutant::Release", "Thread ", currentThread, " attempted to release a mutant owned by thread ", ownerThreadId);
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
                return STATUS_SUCCESS;
            }
            else if (timeout == INFINITE)
            {
                cv.wait(lock, [this]()
                        { return signaled; });

                signaled = false;
                return STATUS_SUCCESS;
            }
            else
            {
                assert(false && "Unsupported timeout value");
                return STATUS_TIMEOUT;
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

    struct FindHandle : KernelObject
    {
        std::error_code ec;
        ankerl::unordered_dense::map<std::u8string, std::pair<size_t, bool>> searchResult; // Relative path, file size, is directory
        decltype(searchResult)::iterator iterator;

        FindHandle(const std::string_view &path)
        {
            auto addDirectory = [&](const std::filesystem::path &directory)
            {
                for (auto &entry : std::filesystem::directory_iterator(directory, ec))
                {
                    std::u8string relativePath = entry.path().lexically_relative(directory).u8string();
                    searchResult.emplace(relativePath, std::make_pair(entry.is_directory(ec) ? 0 : entry.file_size(ec), entry.is_directory(ec)));
                }
            };

            std::string_view pathNoPrefix = path;
            size_t index = pathNoPrefix.find(":\\");

            if (index != std::string_view::npos)
                pathNoPrefix.remove_prefix(index + 2);

            addDirectory(ResolvePath(std::string(path)));

            iterator = searchResult.begin();
        }

        void fillFindData(WIN32_FIND_DATAA *lpFindFileData)
        {
            if (iterator->second.second)
                lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_DIRECTORY);
            else
                lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_NORMAL);

            strncpy(lpFindFileData->cFileName, (const char *)(iterator->first.c_str()), sizeof(lpFindFileData->cFileName));
            lpFindFileData->nFileSizeLow = ByteSwap(uint32_t(iterator->second.first >> 32U));
            lpFindFileData->nFileSizeHigh = ByteSwap(uint32_t(iterator->second.first));
            lpFindFileData->ftCreationTime = {};
            lpFindFileData->ftLastAccessTime = {};
            lpFindFileData->ftLastWriteTime = {};
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