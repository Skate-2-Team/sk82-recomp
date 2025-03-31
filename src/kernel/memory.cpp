#include "memory.h"

// Most of this was copied from Unleashed Recompiled, all credits
namespace Memory
{
    void Init()
    {
        if (g_base == nullptr)
        {
            g_base = (uint8_t *)VirtualAlloc((LPVOID)HOST_PREFERRED_BASE, PPC_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            if (g_base == nullptr)
            {
                g_base = (uint8_t *)VirtualAlloc(nullptr, PPC_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                Log::Error("Kernel->Memory", "Could not get base 0x", HOST_PREFERRED_BASE, ", falling back to -> 0x", (void *)g_base);
            }

            Log::Info("Kernel->Memory", "Memory has been allocated.");

            DWORD oldProtect;
            VirtualProtect(g_base, 4096, PAGE_NOACCESS, &oldProtect);
        }

        for (size_t i = 0; PPCFuncMappings[i].guest != 0; i++)
        {
            if (PPCFuncMappings[i].host != nullptr)
                InsertFunction(PPCFuncMappings[i].guest, PPCFuncMappings[i].host);
        }
    }

    void *Translate(size_t offset)
    {
        if (g_base == nullptr)
            Init();

        if (offset > PPC_MEMORY_SIZE)
            Log::Error("Kernel->Memory", "Offset -> ", (void *)offset, " is wrong.");

        void *result = g_base + offset;

        // Log::Info("Kernel-Memory", "Translated offset -> ", (void *)offset, " to -> ", result);

        return result;
    }

    uint32_t MapVirtual(const void *hostAddress)
    {
        return static_cast<uint32_t>(static_cast<const uint8_t *>(hostAddress) - g_base);
    }

    PPCFunc *FindFunction(uint32_t guest) noexcept
    {
        return PPC_LOOKUP_FUNC(g_base, guest);
    }

    void InsertFunction(uint32_t guest, PPCFunc *host)
    {
        PPC_LOOKUP_FUNC(g_base, guest) = host;
    }
}
