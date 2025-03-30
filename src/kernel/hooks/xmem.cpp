#include "hooks.h"

namespace Hooks
{
    uint32_t Hooks_RtlAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t size)
    {
        void *ptr = g_heap->Alloc(size);
        if ((flags & 0x8) != 0)
            memset(ptr, 0, size);

        // assert(ptr);

        if (ptr == nullptr)
        {
            DebugBreak();
        }

        return Memory::MapVirtual(ptr);
    }

    uint32_t Hooks_RtlReAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer, uint32_t size)
    {
        void *ptr = g_heap->Alloc(size);
        if ((flags & 0x8) != 0)
            memset(ptr, 0, size);

        if (memoryPointer != 0)
        {
            void *oldPtr = Memory::Translate(memoryPointer);
            memcpy(ptr, oldPtr, std::min<size_t>(size, Heap::Size(oldPtr)));
            g_heap->Free(oldPtr);
        }

        assert(ptr);

        return Memory::MapVirtual(ptr);
    }

    uint32_t Hooks_RtlFreeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
    {
        if (memoryPointer != NULL)
            g_heap->Free(Memory::Translate(memoryPointer));

        return true;
    }

    uint32_t Hooks_RtlSizeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
    {
        if (memoryPointer != NULL)
            return (uint32_t)Heap::Size(Memory::Translate(memoryPointer));

        return 0;
    }

    uint32_t Import_MmAllocatePhysicalMemoryEx(
        uint32_t flags,
        uint32_t size,
        uint32_t protect,
        uint32_t minAddress,
        uint32_t maxAddress,
        uint32_t alignment)
    {
        Log::Info("MmAllocatePhysicalMemoryEx", "allocating ", size, " bytes, and ", alignment, " alignment.");
        return Memory::MapVirtual(g_heap->AllocPhysical(size, alignment));
    }

    void Import_MmFreePhysicalMemory(DWORD type, DWORD baseAddress)
    {
        Log::Info("MmFreePhysicalMemory", "Unloading memory in ", baseAddress);
        g_heap->Free(Memory::Translate(baseAddress));
    }

    void Import_NtQueryVirtualMemory()
    {
        Log::Stub("NtQueryVirtualMemory", "Called.");
    }

    void Import_NtAllocateVirtualMemory()
    {
        Log::Stub("NtAllocateVirtualMemory", "Called.");
    }

    void Import_NtFreeVirtualMemory()
    {
        Log::Stub("NtFreeVirtualMemory", "Called.");
    }

    void Import_RtlFillMemoryUlong()
    {
        Log::Stub("RtlFillMemoryUlong", "Called.");
    }

    void Import_RtlCompareMemoryUlong()
    {
        Log::Stub("RtlCompareMemoryUlong", "Called.");
    }

    void Import_MmQueryStatistics()
    {
        Log::Stub("MmQueryStatistics", "Called.");
    }

    void Import_MmQueryAddressProtect()
    {
        Log::Stub("MmQueryAddressProtect", "Called.");
    }

    void Import_MmGetPhysicalAddress()
    {
        Log::Stub("MmGetPhysicalAddress", "Called.");
    }

    void Import_MmSetAddressProtect()
    {
        Log::Stub("MmSetAddressProtect", "Called.");
    }

    void Import_NtProtectVirtualMemory()
    {
        Log::Stub("NtProtectVirtualMemory", "Called.");
    }

    void Import_ExAllocatePoolTypeWithTag()
    {
        Log::Stub("ExAllocatePoolTypeWithTag", "Called.");
    }

    void Import_ExFreePool()
    {
        Log::Stub("ExFreePool", "Called.");
    }
}

// GUEST_FUNCTION_HOOK(sub_82104F80, Hooks::Hooks_RtlAllocateHeap)
// GUEST_FUNCTION_HOOK(sub_82105868, Hooks::Hooks_RtlFreeHeap)
// GUEST_FUNCTION_HOOK(sub_82105B50, Hooks::Hooks_RtlReAllocateHeap)
// GUEST_FUNCTION_HOOK(sub_82104218, Hooks::Hooks_RtlSizeHeap)

GUEST_FUNCTION_HOOK(__imp__ExAllocatePoolTypeWithTag, Hooks::Import_ExAllocatePoolTypeWithTag)
GUEST_FUNCTION_HOOK(__imp__ExFreePool, Hooks::Import_ExFreePool)

GUEST_FUNCTION_HOOK(__imp__MmSetAddressProtect, Hooks::Import_MmSetAddressProtect)
GUEST_FUNCTION_HOOK(__imp__NtProtectVirtualMemory, Hooks::Import_NtProtectVirtualMemory)
GUEST_FUNCTION_HOOK(__imp__MmGetPhysicalAddress, Hooks::Import_MmGetPhysicalAddress)
GUEST_FUNCTION_HOOK(__imp__MmAllocatePhysicalMemoryEx, Hooks::Import_MmAllocatePhysicalMemoryEx)
GUEST_FUNCTION_HOOK(__imp__MmFreePhysicalMemory, Hooks::Import_MmFreePhysicalMemory)
GUEST_FUNCTION_HOOK(__imp__MmQueryStatistics, Hooks::Import_MmQueryStatistics)
GUEST_FUNCTION_HOOK(__imp__MmQueryAddressProtect, Hooks::Import_MmQueryAddressProtect)

GUEST_FUNCTION_HOOK(__imp__NtQueryVirtualMemory, Hooks::Import_NtQueryVirtualMemory)
GUEST_FUNCTION_HOOK(__imp__NtAllocateVirtualMemory, Hooks::Import_NtAllocateVirtualMemory)
GUEST_FUNCTION_HOOK(__imp__NtFreeVirtualMemory, Hooks::Import_NtFreeVirtualMemory)

GUEST_FUNCTION_HOOK(__imp__RtlFillMemoryUlong, Hooks::Import_RtlFillMemoryUlong)
GUEST_FUNCTION_HOOK(__imp__RtlCompareMemoryUlong, Hooks::Import_RtlCompareMemoryUlong)