#include <iostream>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/function.h"
#include "xex.h"

unsigned int Patch_GetTickCount()
{
    return GetTickCount();
}

// recomp is broken for this func
GUEST_FUNCTION_STUB(sub_828F0A10) // Fesl::AchievementManagerImpl::SyncTo360Achievements
GUEST_FUNCTION_STUB(sub_8293D608) // rw::physics::Simulation::HorusPipelineIslandUsingIndirectJacobians

GUEST_FUNCTION_HOOK(sub_82C74580, Patch_GetTickCount) // GetTickCount
