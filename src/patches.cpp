#include <iostream>

#include "ppc/ppc_recomp_shared.h"
#include "kernel/memory.h"
#include "kernel/function.h"
#include "xex.h"

int InputPatch()
{
    return 1;
}

/*
Fesl::Xb360ConnectionImpl::InternalIdle

This function waits for XBL connection, which obviously is never going to happen.
*/
GUEST_FUNCTION_STUB(sub_828CE738)

// Lua lexer has issues, could potentially be because of missing opcodes
GUEST_FUNCTION_STUB(sub_82B33D80) // Sk8::Challenge::cChallengeObjectivesManager::PerformLuaGlobalBinding
GUEST_FUNCTION_STUB(sub_82896278) // ChallengeRuntime::cLuaStateManager::Init

GUEST_FUNCTION_STUB(sub_82368818)             // xcore::XAllocator::Internal::CategoryAllocator::Free
GUEST_FUNCTION_HOOK(sub_8231E2F0, InputPatch) // Sk8::Input::InputManager::LastInput
