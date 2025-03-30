#pragma once

#include <windows.h>
#include <iostream>
#include <profileapi.h>
#include <algorithm>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/heap.h"
#include "kernel/function.h"
#include "kernel/xdm.h"
#include "cpu/guest_thread.h"

#include "kernel/xbox.h"
#include "xex.h"

namespace Hooks
{
    // structs
    // events
    enum NTSTATUS
    {
        STATUS_SUCCESS = 0,
        sSTATUS_TIMEOUT = 1,
        STATUS_UNSUCCESSFUL = -1
    };

    struct Event
    { /* might need some more implementation :p */
        std::atomic<bool> signaled{false};
        std::mutex mtx;
        std::condition_variable cv;
    };

    struct FileHandle : public KernelObject
    {
        FileHandle(HANDLE handle) : m_handle(handle) {};

        ~FileHandle() override
        {
            if (m_handle != INVALID_HANDLE_VALUE)
            {
                Log::Info("FileHandle", "Closing handle -> ", m_handle);
                CloseHandle(m_handle);
            }
        }

        HANDLE m_handle;
    };

    // global vars
    // threading
    inline std::vector<size_t> g_tlsFreeIndices;
    inline size_t g_tlsNextIndex = 0;
    inline Mutex g_tlsAllocationMutex;

    // filesystem
    inline std::vector<HANDLE> g_openedHandles{};
}