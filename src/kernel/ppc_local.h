#pragma once

#include "ppc/ppc_context.h"

namespace PPCLocal
{
    // Makes a PPC context available to every thread.
    inline thread_local PPCContext *g_ppcContext;

    inline PPCContext *GetContext()
    {
        return g_ppcContext;
    }

    inline void SetContext(PPCContext &ctx)
    {
        g_ppcContext = &ctx;
    }

    inline void NewContext()
    {
        if (g_ppcContext == nullptr)
            g_ppcContext = new PPCContext();
    }
}
