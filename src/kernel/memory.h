#pragma once

#include <windows.h>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "ppc/ppc_recomp_shared.h"
#include "log.h"

#define HOST_PREFERRED_BASE 0x100000000ull

namespace Memory
{
    inline uint8_t *g_base;

    void Init();
    void *Translate(size_t offset);
    uint32_t MapVirtual(const void *hostAddress);

    template <typename T>
    T Translate(size_t offset)
    {
        return reinterpret_cast<T>(Translate(offset));
    }

    PPCFunc *FindFunction(uint32_t guest) noexcept;
    void InsertFunction(uint32_t guest, PPCFunc *host);

    inline void *MmGetHostAddress(uint32_t ptr)
    {
        return Translate(ptr);
    }
}

extern PPCFuncMapping PPCFuncMappings[];