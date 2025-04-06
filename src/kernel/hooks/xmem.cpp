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

    // First two args are always NULL every time this is called.
    uint32_t Hooks_VirtualAllocEx(void *hProcess, void *lpAddress, unsigned int dwSize, unsigned int flAllocationType, unsigned int flProtect)
    {
        if (lpAddress != NULL)
        {
            Log::Error("VirtualAllocEx", "lpAddress is not NULL -> ", lpAddress);
            return 0;
        }

        uint32_t address = Memory::MapVirtual(g_heap->Alloc(dwSize));

        Log::Info("VirtualAllocEx", "Alloc -> ", dwSize, " bytes at ", (void *)address);

        return address;
    }

    // If we don't implement this, the EA allocator tries to allocate 4gb of heap space
    void Import_MmQueryStatistics(_MM_STATISTICS *statistics)
    {
        // Ensure the statistics pointer is valid
        if (!statistics)
        {
            Log::Error("MmQueryStatistics", "Invalid statistics pointer.");
            return;
        }

        statistics->Length = sizeof(_MM_STATISTICS);

        // Total Physical Memory: 512MB
        statistics->TotalPhysicalPages = 131072;

        // Kernel Memory: 32MB
        statistics->KernelPages = 8192;

        // Title Available Memory: 256MB
        statistics->TitleAvailablePages = 65536;
        statistics->TitleTotalVirtualMemoryBytes = 536870912;    // 512MB
        statistics->TitleReservedVirtualMemoryBytes = 134217728; // 128MB
        // Title Physical Memory: 240MB
        statistics->TitlePhysicalPages = 61440;

        // Title Memory Breakdown
        statistics->TitlePoolPages = 5120;     // 20MB
        statistics->TitleStackPages = 2048;    // 8MB
        statistics->TitleImagePages = 30720;   // 120MB
        statistics->TitleHeapPages = 15360;    // 60MB
        statistics->TitleVirtualPages = 4096;  // 16MB
        statistics->TitlePageTablePages = 512; // 2MB
        statistics->TitleCachePages = 3584;    // 14MB

        // System Available Memory: 224MB
        statistics->SystemAvailablePages = 57344;
        statistics->SystemTotalVirtualMemoryBytes = 402653184;    // 384MB
        statistics->SystemReservedVirtualMemoryBytes = 100663296; // 96MB

        // System Physical Memory: 208MB
        statistics->SystemPhysicalPages = 53248;

        // System Memory Breakdown
        statistics->SystemPoolPages = 10240;    // 40MB
        statistics->SystemStackPages = 1024;    // 4MB
        statistics->SystemImagePages = 22016;   // 86MB
        statistics->SystemHeapPages = 8192;     // 32MB
        statistics->SystemVirtualPages = 3072;  // 12MB
        statistics->SystemPageTablePages = 512; // 2MB
        statistics->SystemCachePages = 8192;    // 32MB

        // Set the highest physical page (total RAM in pages - 1)
        statistics->HighestPhysicalPage = statistics->TotalPhysicalPages - 1;
    }

    void Import_MmQueryAddressProtect()
    {
        // Log::Stub("MmQueryAddressProtect", "Called.");
    }

    void Import_MmGetPhysicalAddress()
    {
        Log::Stub("MmGetPhysicalAddress", "Called.");
    }

    void Import_MmSetAddressProtect()
    {
        // Log::Stub("MmSetAddressProtect", "Called.");
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

    void Import_NtQueryVirtualMemory()
    {
        Log::Stub("NtQueryVirtualMemory", "Called.");
    }

    void Import_NtAllocateVirtualMemory()
    {
        Log::Info("NtAllocateVirtualMemory", "Called.");
    }

    void Import_NtFreeVirtualMemory()
    {
        Log::Info("NtFreeVirtualMemory", "TRYING TO FREE.");
    }

    void Import_RtlFillMemoryUlong()
    {
        Log::Stub("RtlFillMemoryUlong", "Called.");
    }

    void Import_RtlCompareMemoryUlong()
    {
        Log::Stub("RtlCompareMemoryUlong", "Called.");
    }
}

GUEST_FUNCTION_HOOK(sub_82C72B88, Hooks::Hooks_VirtualAllocEx)
GUEST_FUNCTION_HOOK(sub_82C77290, Hooks::Hooks_RtlAllocateHeap)
GUEST_FUNCTION_HOOK(sub_82C77B60, Hooks::Hooks_RtlFreeHeap)
GUEST_FUNCTION_HOOK(sub_82C77E50, Hooks::Hooks_RtlReAllocateHeap)
GUEST_FUNCTION_HOOK(sub_82C76530, Hooks::Hooks_RtlSizeHeap)

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