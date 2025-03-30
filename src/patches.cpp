#include <iostream>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/function.h"
#include "xex.h"

unsigned int Patch_GetTickCount()
{
    LARGE_INTEGER tickCount;
    QueryPerformanceCounter(&tickCount);

    return static_cast<unsigned int>(tickCount.QuadPart / 10000);
}

int Sk8GetVersion()
{
    return 0;
}

/*
Fesl::Xb360ConnectionImpl::InternalIdle

This function waits for XBL connection, which obviously is never going to happen.
*/
GUEST_FUNCTION_STUB(sub_828CE738)

/*
Sk8::GetVersion

Tries to pull some sort of version file that doesn't exist in the retail build.
*/
GUEST_FUNCTION_HOOK(sub_824D8210, Sk8GetVersion)

/*
AttribSysUtils::cVaultManager::LoadAttributeSystemVault

Tries to load some skater.vlt file that doesn't exist in the retail build.
*/
GUEST_FUNCTION_STUB(sub_828A2458)

// GUEST_FUNCTION_STUB(sub_829F0F00) // rw::core::filesys::Device::Wait
// GUEST_FUNCTION_STUB(sub_829F0A88) // rw::core::filesys::Device::InsertOp
//  GUEST_FUNCTION_HOOK(sub_82C74580, Patch_GetTickCount)