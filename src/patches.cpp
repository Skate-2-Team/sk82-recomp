#include <iostream>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/function.h"
#include "xex.h"

unsigned int Patch_GetTickCount()
{
    return GetTickCount();
}

int Patch_LastInput()
{
    return 1;
}

// Lua lexer has issues, could potentially be because of missing opcodes
GUEST_FUNCTION_STUB(sub_82B33D80) // Sk8::Challenge::cChallengeObjectivesManager::PerformLuaGlobalBinding
GUEST_FUNCTION_STUB(sub_82896278) // ChallengeRuntime::cLuaStateManager::Init

// recomp is broken for this func
GUEST_FUNCTION_STUB(sub_823D7C90) // NFSMixMap::MixMasterChannels

GUEST_FUNCTION_STUB(sub_828F0A10) // Fesl::AchievementManagerImpl::SyncTo360Achievements

// This func waits forever, not sure what causes it
// GUEST_FUNCTION_HOOK(sub_8231E2F0, Patch_LastInput) // Sk8::Input::InputManager::LastInput

GUEST_FUNCTION_HOOK(sub_82C74580, Patch_GetTickCount) // GetTickCount
