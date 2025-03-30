#include "hooks.h"

namespace Hooks
{
    static uint32_t &KeTlsGetValueRef(size_t index)
    {
        thread_local std::vector<uint32_t> s_tlsValues;

        if (s_tlsValues.size() <= index)
        {
            s_tlsValues.resize(index + 1, 0);
        }

        return s_tlsValues[index];
    }

    uint32_t Import_KeTlsFree(uint32_t dwTlsIndex)
    {
        std::lock_guard<Mutex> lock(g_tlsAllocationMutex);
        g_tlsFreeIndices.push_back(dwTlsIndex);
        return TRUE;
    }

    uint32_t Import_KeTlsGetValue(uint32_t dwTlsIndex)
    {
        return KeTlsGetValueRef(dwTlsIndex);
    }

    uint32_t Import_KeTlsSetValue(uint32_t dwTlsIndex, uint32_t lpTlsValue)
    {
        KeTlsGetValueRef(dwTlsIndex) = lpTlsValue;
        return TRUE;
    }

    uint32_t Import_KeTlsAlloc()
    {
        std::lock_guard<Mutex> lock(g_tlsAllocationMutex);
        if (!g_tlsFreeIndices.empty())
        {
            size_t index = g_tlsFreeIndices.back();
            g_tlsFreeIndices.pop_back();
            return index;
        }

        return g_tlsNextIndex++;
    }

    int Import_ExCreateThread(be<uint32_t> *handle, uint32_t stackSize, be<uint32_t> *threadId, uint32_t xApiThreadStartup,
                              uint32_t startAddress, uint32_t startContext, uint32_t creationFlags)
    {
        // list all the parameters in log
        Log::Info("ExCreateThread",
                  "handle: ", handle,
                  " stackSize: ", stackSize,
                  " threadId: ", threadId,
                  " xApiThreadStartup: ", xApiThreadStartup,
                  " startAddress: ", startAddress,
                  " startContext: ", startContext,
                  " creationFlags: ", creationFlags);

        uint32_t hostThreadId;

        *handle = GetKernelHandle(GuestThread::Start({startAddress, startContext, creationFlags}, &hostThreadId));

        Log::Info("ExCreateThread", "Thread created with handle: ", *handle, " and threadId: ", hostThreadId);

        if (threadId != nullptr)
            *threadId = hostThreadId;

        return 0;
    }

    uint32_t Import_NtResumeThread(GuestThreadHandle *hThread, uint32_t *suspendCount)
    {
        assert(hThread != GetKernelObject(CURRENT_THREAD_HANDLE));

        hThread->suspended = false;
        hThread->suspended.notify_all();

        return S_OK;
    }

    uint32_t Import_KeResumeThread(GuestThreadHandle *object)
    {
        assert(object != GetKernelObject(CURRENT_THREAD_HANDLE));

        object->suspended = false;
        object->suspended.notify_all();
        return 0;
    }

    uint32_t Import_KeSetAffinityThread(uint32_t Thread, uint32_t Affinity, be<uint32_t> *lpPreviousAffinity)
    {
        if (lpPreviousAffinity)
            *lpPreviousAffinity = 2;

        return 0;
    }

    void Import_KeSetBasePriorityThread(GuestThreadHandle *hThread, int priority)
    {
#ifdef _WIN32
        if (priority == 16)
        {
            priority = 15;
        }
        else if (priority == -16)
        {
            priority = -15;
        }

        SetThreadPriority(hThread == GetKernelObject(CURRENT_THREAD_HANDLE) ? GetCurrentThread() : hThread->thread.native_handle(), priority);
#endif
    }

    void Import_RtlEnterCriticalSection(XRTL_CRITICAL_SECTION *cs)
    {
        auto currentThread = PPCLocal::g_ppcContext->r13.u32;

        assert(currentThread != NULL);

        std::atomic_ref owningThread(cs->OwningThread);

        while (true)
        {
            uint32_t previousOwner = 0;

            if (owningThread.compare_exchange_weak(previousOwner, currentThread) || previousOwner == currentThread)
            {
                cs->RecursionCount++;
                return;
            }

            owningThread.wait(previousOwner);
        }
    }

    void Import_RtlLeaveCriticalSection(XRTL_CRITICAL_SECTION *cs)
    {
        cs->RecursionCount--;

        if (cs->RecursionCount != 0)
            return;

        std::atomic_ref owningThread(cs->OwningThread);
        owningThread.store(0);
        owningThread.notify_one();
    }

    void Import_KeWaitForMultipleObjects()
    {
        Log::Stub("KeWaitForMultipleObjects", "Called.");
    }

    void Import_KeInitializeSemaphore()
    {
        Log::Stub("KeInitializeSemaphore", "Called.");
    }

    void Import_KeReleaseSemaphore()
    {
        Log::Stub("KeReleaseSemaphore", "Called.");
    }

    void Import_RtlTryEnterCriticalSection()
    {
        Log::Stub("RtlTryEnterCriticalSection", "Called.");
    }

    void Import_KeTryToAcquireSpinLockAtRaisedIrql()
    {
        Log::Stub("KeTryToAcquireSpinLockAtRaisedIrql", "Called.");
    }

    void Import_RtlInitializeCriticalSection()
    {
        Log::Stub("RtlInitializeCriticalSection", "Called.");
    }

    void Import_KeLockL2()
    {
        Log::Stub("KeLockL2", "Called.");
    }

    void Import_KeUnlockL2()
    {
        Log::Stub("KeUnlockL2", "Called.");
    }

    void Import_KeResetEvent()
    {
        Log::Stub("KeResetEvent", "Called.");
    }

    void Import_KeReleaseSpinLockFromRaisedIrql()
    {
        Log::Stub("KeReleaseSpinLockFromRaisedIrql", "Called.");
    }

    void Import_KeAcquireSpinLockAtRaisedIrql()
    {
        Log::Stub("KeAcquireSpinLockAtRaisedIrql", "Called.");
    }

    void Import_KfReleaseSpinLock()
    {
        Log::Stub("KfReleaseSpinLock", "Called.");
    }

    void Import_KfAcquireSpinLock()
    {
        Log::Stub("KfAcquireSpinLock", "Called.");
    }

    void Import_ExTerminateThread()
    {
        Log::Stub("ExTerminateThread", "Called.");
    }

    DWORD Import_NtCreateEvent(Event **evtOut)
    {
        if (!evtOut)
            return STATUS_UNSUCCESSFUL;
        // Allocate and initialize the event object.
        *evtOut = new Event();
        return STATUS_SUCCESS;
    }

    DWORD Import_NtSetEvent(Event *evt, unsigned long *previousState = nullptr)
    {
        if (!evt)
            return STATUS_UNSUCCESSFUL;

        // Lock the mutex to synchronize with waiters
        std::lock_guard<std::mutex> lock(evt->mtx);
        bool old = evt->signaled.exchange(true, std::memory_order_acq_rel);
        if (previousState)
            *previousState = old ? 1 : 0;

        // Notify all waiting threads to check the predicate
        evt->cv.notify_all(); // Or notify_one() for auto-reset behavior
        return STATUS_SUCCESS;
    }

    DWORD Import_NtClearEvent(Event *evt, unsigned long *previousState = nullptr)
    {
        if (!evt)
            return STATUS_UNSUCCESSFUL;

        std::lock_guard<std::mutex> lock(evt->mtx); // Optional but safe
        bool old = evt->signaled.exchange(false, std::memory_order_acq_rel);
        if (previousState)
            *previousState = old ? 1 : 0;
        return STATUS_SUCCESS;
    }

    uint32_t GuestTimeoutToMilliseconds(be<int64_t> *timeout)
    {
        return timeout ? (*timeout * -1) / 10000 : INFINITE;
    }

    uint32_t Import_NtWaitForSingleObjectEx(uint32_t Handle, uint32_t WaitMode, uint32_t Alertable, be<int64_t> *Timeout)
    {
        uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);
        assert(timeout == 0 || timeout == INFINITE);

        if (IsKernelObject(Handle))
        {
            return GetKernelObject(Handle)->Wait(timeout);
        }
        else
        {
            assert(false && "Unrecognized handle value.");
        }

        return STATUS_TIMEOUT;
    }

    void Import_KeLeaveCriticalRegion()
    {
        Log::Stub("KeLeaveCriticalRegion", "Called.");
    }

    void Import_KeSetEvent()
    {
        Log::Stub("KeSetEvent", "Called.");
    }
    void Import_KeWaitForSingleObject()
    {
        Log::Stub("KeWaitForSingleObject", "Called.");
    }

    void Import_KeEnterCriticalRegion()
    {
        Log::Stub("KeEnterCriticalRegion", "Called.");
    }

    void Import_KeDelayExecutionThread()
    {
        Log::Stub("KeDelayExecutionThread", "Called.");
    }

    void Import_NtReleaseMutant()
    {
        Log::Stub("NtReleaseMutant", "Called.");
    }

    void Import_RtlInitializeCriticalSectionAndSpinCount()
    {
        Log::Stub("RtlInitializeCriticalSectionAndSpinCount", "Called.");
    }

    void Import_NtCreateMutant()
    {
        Log::Stub("NtCreateMutant", "Called.");
    }

    void Import_NtCreateSemaphore()
    {
        Log::Stub("NtCreateSemaphore", "Called.");
    }

    void Import_NtCreateTimer()
    {
        Log::Stub("NtCreateTimer", "Called.");
    }

    void Import_NtReleaseSemaphore()
    {
        Log::Stub("NtReleaseSemaphore", "Called.");
    }

    void Import_NtSetTimerEx()
    {
        Log::Stub("NtSetTimerEx", "Called.");
    }

    void Import_NtYieldExecution()
    {
        Log::Stub("NtYieldExecution", "Called.");
    }

    void Import_NtCancelTimer()
    {
        Log::Stub("NtCancelTimer", "Called.");
    }
}

GUEST_FUNCTION_HOOK(__imp__NtCancelTimer, Hooks::Import_NtCancelTimer)
GUEST_FUNCTION_HOOK(__imp__NtCreateMutant, Hooks::Import_NtCreateMutant)
GUEST_FUNCTION_HOOK(__imp__NtCreateSemaphore, Hooks::Import_NtCreateSemaphore)
GUEST_FUNCTION_HOOK(__imp__NtCreateTimer, Hooks::Import_NtCreateTimer)
GUEST_FUNCTION_HOOK(__imp__NtReleaseSemaphore, Hooks::Import_NtReleaseSemaphore)
GUEST_FUNCTION_HOOK(__imp__NtSetTimerEx, Hooks::Import_NtSetTimerEx)
GUEST_FUNCTION_HOOK(__imp__NtYieldExecution, Hooks::Import_NtYieldExecution)
GUEST_FUNCTION_HOOK(__imp__NtReleaseMutant, Hooks::Import_NtReleaseMutant)
GUEST_FUNCTION_HOOK(__imp__NtSetEvent, Hooks::Import_NtSetEvent)
GUEST_FUNCTION_HOOK(__imp__NtClearEvent, Hooks::Import_NtClearEvent)
GUEST_FUNCTION_HOOK(__imp__NtCreateEvent, Hooks::Import_NtCreateEvent)
GUEST_FUNCTION_HOOK(__imp__NtResumeThread, Hooks::Import_NtResumeThread)
GUEST_FUNCTION_HOOK(__imp__NtWaitForSingleObjectEx, Hooks::Import_NtWaitForSingleObjectEx)

GUEST_FUNCTION_HOOK(__imp__ExTerminateThread, Hooks::Import_ExTerminateThread)
GUEST_FUNCTION_HOOK(__imp__ExCreateThread, Hooks::Import_ExCreateThread)

GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSectionAndSpinCount, Hooks::Import_RtlInitializeCriticalSectionAndSpinCount)
GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSection, Hooks::Import_RtlInitializeCriticalSection)
GUEST_FUNCTION_HOOK(__imp__RtlTryEnterCriticalSection, Hooks::Import_RtlTryEnterCriticalSection)
GUEST_FUNCTION_HOOK(__imp__RtlEnterCriticalSection, Hooks::Import_RtlEnterCriticalSection)
GUEST_FUNCTION_HOOK(__imp__RtlLeaveCriticalSection, Hooks::Import_RtlLeaveCriticalSection)

GUEST_FUNCTION_HOOK(__imp__KfAcquireSpinLock, Hooks::Import_KfAcquireSpinLock)
GUEST_FUNCTION_HOOK(__imp__KfReleaseSpinLock, Hooks::Import_KfReleaseSpinLock)

GUEST_FUNCTION_HOOK(__imp__KeWaitForMultipleObjects, Hooks::Import_KeWaitForMultipleObjects)
GUEST_FUNCTION_HOOK(__imp__KeInitializeSemaphore, Hooks::Import_KeInitializeSemaphore)
GUEST_FUNCTION_HOOK(__imp__KeReleaseSemaphore, Hooks::Import_KeReleaseSemaphore)
GUEST_FUNCTION_HOOK(__imp__KeTryToAcquireSpinLockAtRaisedIrql, Hooks::Import_KeTryToAcquireSpinLockAtRaisedIrql)
GUEST_FUNCTION_HOOK(__imp__KeAcquireSpinLockAtRaisedIrql, Hooks::Import_KeAcquireSpinLockAtRaisedIrql)
GUEST_FUNCTION_HOOK(__imp__KeReleaseSpinLockFromRaisedIrql, Hooks::Import_KeReleaseSpinLockFromRaisedIrql)
GUEST_FUNCTION_HOOK(__imp__KeWaitForSingleObject, Hooks::Import_KeWaitForSingleObject)
GUEST_FUNCTION_HOOK(__imp__KeResetEvent, Hooks::Import_KeResetEvent)
GUEST_FUNCTION_HOOK(__imp__KeUnlockL2, Hooks::Import_KeUnlockL2)
GUEST_FUNCTION_HOOK(__imp__KeLockL2, Hooks::Import_KeLockL2)
GUEST_FUNCTION_HOOK(__imp__KeSetBasePriorityThread, Hooks::Import_KeSetBasePriorityThread)
GUEST_FUNCTION_HOOK(__imp__KeSetAffinityThread, Hooks::Import_KeSetAffinityThread)
GUEST_FUNCTION_HOOK(__imp__KeResumeThread, Hooks::Import_KeResumeThread)
GUEST_FUNCTION_HOOK(__imp__KeTlsGetValue, Hooks::Import_KeTlsGetValue)
GUEST_FUNCTION_HOOK(__imp__KeTlsSetValue, Hooks::Import_KeTlsSetValue)
GUEST_FUNCTION_HOOK(__imp__KeTlsAlloc, Hooks::Import_KeTlsAlloc)
GUEST_FUNCTION_HOOK(__imp__KeTlsFree, Hooks::Import_KeTlsFree)
GUEST_FUNCTION_HOOK(__imp__KeDelayExecutionThread, Hooks::Import_KeDelayExecutionThread)
GUEST_FUNCTION_HOOK(__imp__KeEnterCriticalRegion, Hooks::Import_KeEnterCriticalRegion)
GUEST_FUNCTION_HOOK(__imp__KeSetEvent, Hooks::Import_KeSetEvent)
GUEST_FUNCTION_HOOK(__imp__KeLeaveCriticalRegion, Hooks::Import_KeLeaveCriticalRegion)
