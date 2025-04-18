#include "hooks.h"

// Again thanks to Unleashed Recompiled for majority of the original implementations <3
// https://github.com/hedge-dev/UnleashedRecomp/blob/main/UnleashedRecomp/kernel/imports.cpp
// all credits where credits are due <3

namespace Hooks
{
    static std::atomic<uint32_t> g_keSetEventGeneration;

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

    static std::map<XRTL_CRITICAL_SECTION *, uint32_t> g_criticalSections;
    static std::mutex g_criticalSectionsMutex;

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

                {
                    std::lock_guard<std::mutex> lock(g_criticalSectionsMutex);
                    g_criticalSections[cs] = GetCurrentThreadId();
                }

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

        {
            std::lock_guard<std::mutex> lock(g_criticalSectionsMutex);
            g_criticalSections.erase(cs);
        }

        std::atomic_ref owningThread(cs->OwningThread);
        owningThread.store(0);
        owningThread.notify_one();
    }

    bool Import_RtlTryEnterCriticalSection(XRTL_CRITICAL_SECTION *cs)
    {
        uint32_t thisThread = PPCLocal::g_ppcContext->r13.u32;
        assert(thisThread != NULL);

        std::atomic_ref owningThread(cs->OwningThread);

        uint32_t previousOwner = 0;

        if (owningThread.compare_exchange_weak(previousOwner, thisThread) || previousOwner == thisThread)
        {
            cs->RecursionCount++;

            {
                std::lock_guard<std::mutex> lock(g_criticalSectionsMutex);
                g_criticalSections[cs] = GetCurrentThreadId();
            }

            return true;
        }

        {
            // print out
            std::lock_guard<std::mutex> lock(g_criticalSectionsMutex);

            Log::Info("RtlTryEnterCriticalSection", "Thread ", GetCurrentThreadId(), " failed to enter critical section owned by thread ", g_criticalSections[cs]);
        }

        return false;
    }

    uint32_t Import_RtlInitializeCriticalSection(XRTL_CRITICAL_SECTION *cs)
    {
        cs->Header.Absolute = 0;
        cs->LockCount = -1;
        cs->RecursionCount = 0;
        cs->OwningThread = 0;

        return 0;
    }

    bool Import_KeSetEvent(XKEVENT *pEvent, uint32_t Increment, bool Wait)
    {

        bool result = QueryKernelObject<Event>(*pEvent)->Set();

        ++g_keSetEventGeneration;
        g_keSetEventGeneration.notify_all();

        return result;
    }

    bool Import_KeResetEvent(XKEVENT *pEvent)
    {
        return QueryKernelObject<Event>(*pEvent)->Reset();
    }

    uint32_t Import_KeWaitForSingleObject(XDISPATCHER_HEADER *Object, uint32_t WaitReason, uint32_t WaitMode, bool Alertable, be<int64_t> *Timeout)
    {
        const uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);
        assert(timeout == INFINITE);

        switch (Object->Type)
        {
        case 0:
        case 1:
            QueryKernelObject<Event>(*Object)->Wait(timeout);
            break;

        case 5:
            QueryKernelObject<Semaphore>(*Object)->Wait(timeout);
            break;

        default:
            assert(false && "Unrecognized kernel object type.");
            return STATUS_TIMEOUT;
        }

        return STATUS_SUCCESS;
    }

    void Import_ExTerminateThread()
    {
        ExitThread(0);
    }

    // Game uses this to see if the video decoder thread is still alive, otherwise it wont
    // start the next movie
    int Hooks_GetExitCodeThread(uint32_t hThread, unsigned int *lpExitCode)
    {
        // thread address comes out of heap struct
        hThread = ByteSwap(hThread);

        auto threadContext = Memory::Translate<GuestThreadHandle *>(hThread);

        HANDLE curHandle;

        if (threadContext == GetKernelObject(CURRENT_THREAD_HANDLE))
            curHandle = GetCurrentThread();
        else
            curHandle = threadContext->thread.native_handle();

        DWORD localExit;

        BOOL result = GetExitCodeThread(curHandle, &localExit);

        *lpExitCode = ByteSwap(localExit);

        return result;
    }

    uint32_t Import_NtCreateEvent(be<uint32_t> *handle, void *objAttributes, uint32_t eventType, uint32_t initialState)
    {
        *handle = GetKernelHandle(CreateKernelObject<Event>(!eventType, !!initialState));
        return 0;
    }

    uint32_t Import_NtSetEvent(Event *handle, uint32_t *previousState)
    {
        handle->Set();
        return 0;
    }

    uint32_t Import_NtClearEvent(Event *handle, uint32_t *previousState)
    {
        handle->Reset();
        return 0;
    }

    uint32_t Import_NtWaitForSingleObjectEx(uint32_t Handle, uint32_t WaitMode, uint32_t Alertable, be<int64_t> *Timeout)
    {
        uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);

        // No need for timeout check because the Wait() assert will catch it
        if (IsKernelObject(Handle))
        {
            return GetKernelObject(Handle)->Wait(timeout);
        }
        else
        {
            Log::Error("NtWaitForSingleObjectEx", "Handle not recognized -> ", Handle);
            DebugBreak();
        }

        return STATUS_TIMEOUT;
    }

    uint32_t Import_KeDelayExecutionThread(uint32_t WaitMode, bool Alertable, be<int64_t> *Timeout)
    {
        uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);

#ifdef _WIN32
        Sleep(timeout);
#else
        if (timeout == 0)
            std::this_thread::yield();
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
#endif

        return STATUS_SUCCESS;
    }

    uint32_t Import_NtReleaseMutant(Mutant *handle)
    {
        handle->Release();
        return STATUS_SUCCESS;
    }

    void Import_RtlInitializeCriticalSectionAndSpinCount(XRTL_CRITICAL_SECTION *cs, uint32_t spinCount)
    {
        cs->Header.Absolute = (spinCount + 255) >> 8;
        cs->LockCount = -1;
        cs->RecursionCount = 0;
        cs->OwningThread = 0;
    }

    uint32_t Import_NtCreateMutant(be<uint32_t> *handle, void *objAttributes, uint32_t initialOwner)
    {
        *handle = GetKernelHandle(CreateKernelObject<Mutant>(!!initialOwner));
        return STATUS_SUCCESS;
    }

    uint32_t Import_NtCreateSemaphore(be<uint32_t> *Handle, XOBJECT_ATTRIBUTES *ObjectAttributes, uint32_t InitialCount, uint32_t MaximumCount)
    {
        *Handle = GetKernelHandle(CreateKernelObject<Semaphore>(InitialCount, MaximumCount));
        return STATUS_SUCCESS;
    }

    uint32_t Import_NtReleaseSemaphore(Semaphore *Handle, uint32_t ReleaseCount, int32_t *PreviousCount)
    {
        uint32_t previousCount;
        Handle->Release(ReleaseCount, &previousCount);

        if (PreviousCount != nullptr)
            *PreviousCount = ByteSwap(previousCount);

        return STATUS_SUCCESS;
    }

    uint32_t Import_NtCreateTimer(be<uint32_t> *handle, void *objAttributes, uint32_t timerType)
    {
        bool autoReset = (timerType != 0);

        *handle = GetKernelHandle(CreateKernelObject<Timer>(autoReset));

        return STATUS_SUCCESS;
    }

    uint32_t Import_NtSetTimerEx(
        Timer *timer,
        const _LARGE_INTEGER *lpDueTime,
        int unused1,
        int unused2,
        int unused3,
        int unused4,
        int lPeriod)
    {
        if (!timer)
            return 0;

        int64_t dueTime100ns = lpDueTime ? ByteSwap(lpDueTime->QuadPart) : 0;
        uint32_t dueTimeMs = 0;

        if (dueTime100ns < 0)
        {
            dueTimeMs = static_cast<uint32_t>((-dueTime100ns) / 10000);
        }
        else if (dueTime100ns > 0)
        {
            int64_t currentTime100ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count() /
                                       100;

            if (dueTime100ns <= currentTime100ns)
            {
                dueTimeMs = 0;
            }
            else
            {
                dueTimeMs = static_cast<uint32_t>((dueTime100ns - currentTime100ns) / 10000);
            }
        }
        else
        {
            dueTimeMs = 0;
        }

        timer->SetTimer(dueTimeMs, static_cast<uint32_t>(lPeriod));

        return 0;
    }

    void Import_NtCancelTimer(Timer *timer)
    {
        if (!timer)
            return;

        timer->CancelTimer();
    }

    void Import_NtYieldExecution()
    {
        std::this_thread::yield();
    }

    uint32_t Import_NtDuplicateObject(uint32_t hSourceHandle, void **lpTargetHandle, uint32_t dwOptions)
    {
        if (hSourceHandle == CURRENT_THREAD_HANDLE)
        {
            GuestThreadHandle *currentThread = GetKernelObject<GuestThreadHandle>(CURRENT_THREAD_HANDLE);

            *lpTargetHandle = (void *)GetKernelHandle(currentThread);

            Log::Info("NtDuplicateObject", "Duplicated current thread handle: target=", (uint32_t)*lpTargetHandle);

            return STATUS_SUCCESS;
        }
        else if (IsKernelObject(hSourceHandle))
        {
            GuestThreadHandle *sourceThread = GetKernelObject<GuestThreadHandle>(hSourceHandle);

            if (sourceThread != nullptr && !IsInvalidKernelObject(sourceThread))
            {
                *lpTargetHandle = (void *)GetKernelHandle(sourceThread);

                Log::Info("NtDuplicateObject", "Duplicated handle: source=", hSourceHandle, " target=", (uint32_t)*lpTargetHandle);

                return STATUS_SUCCESS;
            }
        }

        Log::Error("NtDuplicateObject", "Failed to duplicate handle: ", hSourceHandle);
        DebugBreak();

        return STATUS_INVALID_HANDLE;
    }

    void __fastcall Hooks_RaiseException(unsigned int dwExceptionCode, char dwExceptionFlags, unsigned int nNumberOfArguments, char *lpArguments)
    {
        if (dwExceptionCode == 0x406D1388) // Set Thread Name
        {
            THREADNAME_INFO *threadNameInfo = (THREADNAME_INFO *)lpArguments;

            if (threadNameInfo->dwType == 0x1000)
            {
                char *threadName = Memory::Translate<char *>(threadNameInfo->szName.get());

                // Set thread name in TEB
                GuestThread::SetThreadName(threadNameInfo->dwThreadID, threadName);
            }
        }
    }

    // Unleashed recompiled states this function only handles event objects, which is fine because we only use events.
    uint32_t Import_KeWaitForMultipleObjects(uint32_t Count, xpointer<XDISPATCHER_HEADER> *Objects, uint32_t WaitType, uint32_t WaitReason, uint32_t WaitMode, uint32_t Alertable, be<int64_t> *Timeout)
    {
        const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);
        assert(timeout == INFINITE);

        if (WaitType == 0) // Wait all
        {
            for (size_t i = 0; i < Count; i++)
                QueryKernelObject<Event>(*Objects[i])->Wait(timeout);
        }
        else
        {
            thread_local std::vector<Event *> s_events;
            s_events.resize(Count);

            for (size_t i = 0; i < Count; i++)
                s_events[i] = QueryKernelObject<Event>(*Objects[i]);

            while (true)
            {
                uint32_t generation = g_keSetEventGeneration.load();

                for (size_t i = 0; i < Count; i++)
                {
                    if (s_events[i]->Wait(0) == STATUS_SUCCESS)
                    {
                        return STATUS_WAIT_0 + i;
                    }
                }

                g_keSetEventGeneration.wait(generation);
            }
        }

        return STATUS_SUCCESS;
    }

    void Import_KeInitializeSemaphore(XKSEMAPHORE *semaphore, uint32_t count, uint32_t limit)
    {
        semaphore->Header.Type = 5;
        semaphore->Header.SignalState = count;
        semaphore->Limit = limit;

        auto *object = QueryKernelObject<Semaphore>(semaphore->Header);
    }

    uint32_t Import_KeReleaseSemaphore(XKSEMAPHORE *semaphore, uint32_t increment, uint32_t adjustment, uint32_t wait)
    {
        auto *object = QueryKernelObject<Semaphore>(semaphore->Header);
        object->Release(adjustment, nullptr);
        return STATUS_SUCCESS;
    }

    void Import_KeReleaseSpinLockFromRaisedIrql(uint32_t *spinLock)
    {
        std::atomic_ref spinLockRef(*spinLock);
        spinLockRef = 0;
    }

    void Import_KeAcquireSpinLockAtRaisedIrql(uint32_t *spinLock)
    {
        std::atomic_ref spinLockRef(*spinLock);

        while (true)
        {
            uint32_t expected = 0;
            if (spinLockRef.compare_exchange_weak(expected, PPCLocal::g_ppcContext->r13.u32))
                break;

            std::this_thread::yield();
        }
    }

    void Import_KfReleaseSpinLock(uint32_t *spinLock)
    {
        std::atomic_ref spinLockRef(*spinLock);
        spinLockRef = 0;
    }

    void Import_KfAcquireSpinLock(uint32_t *spinLock)
    {
        std::atomic_ref spinLockRef(*spinLock);

        while (true)
        {
            uint32_t expected = 0;
            if (spinLockRef.compare_exchange_weak(expected, PPCLocal::g_ppcContext->r13.u32))
                break;

            std::this_thread::yield();
        }
    }

    // Used by D3D functions
    void Import_KeLockL2()
    {
        Log::Stub("KeLockL2", "Called.");
    }

    void Import_KeUnlockL2()
    {
        Log::Stub("KeUnlockL2", "Called.");
    }

    // Used by MountUtilityDrive
    void Import_KeLeaveCriticalRegion()
    {
        Log::Stub("KeLeaveCriticalRegion", "Called.");
    }

    void Import_KeEnterCriticalRegion()
    {
        Log::Stub("KeEnterCriticalRegion", "Called.");
    }

    // Last Errors
    int Hooks_RtlSetLastNTError(int Status)
    {
        return 0;
    }

    void Hooks_RtlSetLastError(int dwErrCode)
    {
        return;
    }
}

GUEST_FUNCTION_HOOK(sub_82C78FC0, Hooks::Hooks_RtlSetLastNTError)
GUEST_FUNCTION_HOOK(sub_82C78FA8, Hooks::Hooks_RtlSetLastError)

// RaiseException
GUEST_FUNCTION_HOOK(sub_82C78920, Hooks::Hooks_RaiseException)
GUEST_FUNCTION_HOOK(sub_82C74278, Hooks::Hooks_GetExitCodeThread)

GUEST_FUNCTION_HOOK(__imp__NtDuplicateObject, Hooks::Import_NtDuplicateObject)
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
