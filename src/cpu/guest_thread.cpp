#include "guest_thread.h"

#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/function.h>
#include <kernel/ppc_local.h>
#include <cassert>

constexpr size_t PCR_SIZE = 0xAB0;
constexpr size_t TLS_SIZE = 0x100;
constexpr size_t TEB_SIZE = 0x2E0;
constexpr size_t STACK_SIZE = 0x40000;
constexpr size_t TOTAL_SIZE = PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE;

constexpr size_t TEB_OFFSET = PCR_SIZE + TLS_SIZE;

GuestThreadContext::GuestThreadContext(uint32_t cpuNumber)
{
    assert(thread == nullptr);

    thread = (uint8_t *)g_heap->Alloc(TOTAL_SIZE);
    memset(thread, 0, TOTAL_SIZE);

    *(uint32_t *)thread = ByteSwap(Memory::MapVirtual(thread + PCR_SIZE));                      // tls pointer
    *(uint32_t *)(thread + 0x100) = ByteSwap(Memory::MapVirtual(thread + PCR_SIZE + TLS_SIZE)); // teb pointer
    *(thread + 0x10C) = cpuNumber;

    *(uint32_t *)(thread + PCR_SIZE + 0x10) = 0xFFFFFFFF;                                              // that one TLS entry that felt quirky
    *(uint32_t *)(thread + PCR_SIZE + TLS_SIZE + 0x14C) = ByteSwap(GuestThread::GetCurrentThreadId()); // thread id

    ppcContext.r1.u64 = Memory::MapVirtual(thread + PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE); // stack pointer
    ppcContext.r13.u64 = Memory::MapVirtual(thread);

    ppcContext.fpscr.loadFromHost();

    assert(PPCLocal::GetContext() == nullptr);
    PPCLocal::SetContext(ppcContext);
}

GuestThreadContext::~GuestThreadContext()
{
    g_heap->Free(thread);
}

static void GuestThreadFunc(GuestThreadHandle *hThread)
{
    hThread->suspended.wait(true);
    GuestThread::Start(hThread->params);
}

GuestThreadHandle::GuestThreadHandle(const GuestThreadParams &params)
    : params(params), suspended((params.flags & 0x1) != 0), thread(GuestThreadFunc, this)
{
}

GuestThreadHandle::~GuestThreadHandle()
{
    if (thread.joinable())
        thread.join();
}

uint32_t GuestThreadHandle::Wait(uint32_t timeout)
{
    assert(timeout == INFINITE);

    if (thread.joinable())
        thread.join();

    return STATUS_WAIT_0;
}

uint32_t GuestThread::Start(const GuestThreadParams &params)
{
    const auto procMask = (uint8_t)(params.flags >> 24);
    const auto cpuNumber = procMask == 0 ? 0 : 7 - std::countl_zero(procMask);

    GuestThreadContext ctx(cpuNumber);
    ctx.ppcContext.r3.u64 = params.value;

    Memory::FindFunction(params.function)(ctx.ppcContext, Memory::g_base);

    return ctx.ppcContext.r3.u32;
}

static uint32_t GetThreadId(const std::thread::id &id)
{
    if constexpr (sizeof(id) == 4)
        return *reinterpret_cast<const uint32_t *>(&id);
    else
    {
        Log::Info("GuestThread", "Thread ID is too large.");
        DebugBreak();
     }
}

GuestThreadHandle *GuestThread::Start(const GuestThreadParams &params, uint32_t *threadId)
{
    auto hThread = CreateKernelObject<GuestThreadHandle>(params);

    if (threadId != nullptr)
        *threadId = GetThreadId(hThread->thread.get_id());

    return hThread;
}

uint32_t GuestThread::GetCurrentThreadId()
{
    return GetThreadId(std::this_thread::get_id());
}

void GuestThread::SetLastError(uint32_t error)
{
    auto *thread = (char *)Memory::Translate(PPCLocal::GetContext()->r13.u32);
    if (*(uint32_t *)(thread + 0x150))
    {
        // Program doesn't want errors
        return;
    }

    // TEB + 0x160 : Win32LastError
    *(uint32_t *)(thread + TEB_OFFSET + 0x160) = ByteSwap(error);
}

#ifdef _WIN32
void GuestThread::SetThreadName(uint32_t threadId, const char *name)
{
#pragma pack(push, 8)
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;     // Must be 0x1000.
        LPCSTR szName;    // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags;    // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif
