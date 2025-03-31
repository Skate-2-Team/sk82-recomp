#include "kernel/function.h"
#include "log.h"
#include "kernel/xbox.h"
#include <SDL3/SDL_keyboard.h>

namespace InputHooks
{
    uint32_t GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES *pCaps)
    {
        if (!pCaps)
            return ERROR_BAD_ARGUMENTS;

        memset(pCaps, 0, sizeof(*pCaps));

        XAMINPUT_GAMEPAD Gamepad = {0};
        XAMINPUT_VIBRATION Vibration = {0};

        pCaps->Type = XAMINPUT_DEVTYPE_GAMEPAD;
        pCaps->SubType = XAMINPUT_DEVSUBTYPE_GAMEPAD;
        pCaps->Flags = 0;
        pCaps->Gamepad = Gamepad;
        pCaps->Vibration = Vibration;

        return ERROR_SUCCESS;
    }

    uint32_t Import_XamInputGetCapabilities(uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES *caps)
    {
        uint32_t result = GetCapabilities(userIndex, caps);

        if (result == ERROR_SUCCESS)
        {
            ByteSwapInplace(caps->Flags);
            ByteSwapInplace(caps->Gamepad.wButtons);
            ByteSwapInplace(caps->Gamepad.sThumbLX);
            ByteSwapInplace(caps->Gamepad.sThumbLY);
            ByteSwapInplace(caps->Gamepad.sThumbRX);
            ByteSwapInplace(caps->Gamepad.sThumbRY);
            ByteSwapInplace(caps->Vibration.wLeftMotorSpeed);
            ByteSwapInplace(caps->Vibration.wRightMotorSpeed);
        }

        return result;
    }

    uint32_t Import_XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE *state)
    {
        memset(state, 0, sizeof(*state));

        auto keyboardState = SDL_GetKeyboardState(NULL);

        if (keyboardState[SDL_SCANCODE_UP])
            state->Gamepad.sThumbLY = 32767;
        if (keyboardState[SDL_SCANCODE_DOWN])
            state->Gamepad.sThumbLY = -32768;
        if (keyboardState[SDL_SCANCODE_LEFT])
            state->Gamepad.sThumbLX = -32768;
        if (keyboardState[SDL_SCANCODE_RIGHT])
            state->Gamepad.sThumbLX = 32767;

        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.sThumbRY = 32767;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.sThumbRY = -32768;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.sThumbRX = -32768;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.sThumbRX = 32767;

        if (keyboardState[SDL_SCANCODE_1])
            state->Gamepad.bLeftTrigger = 0xFF;
        if (keyboardState[SDL_SCANCODE_3])
            state->Gamepad.bRightTrigger = 0xFF;

        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_UP;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_DOWN;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        if (keyboardState[SDL_SCANCODE_UNKNOWN])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;

        if (keyboardState[SDL_SCANCODE_RETURN])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_START;
        if (keyboardState[SDL_SCANCODE_BACKSPACE])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_BACK;

        if (keyboardState[SDL_SCANCODE_Q])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
        if (keyboardState[SDL_SCANCODE_E])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;

        if (keyboardState[SDL_SCANCODE_S])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_A;
        if (keyboardState[SDL_SCANCODE_D])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        if (keyboardState[SDL_SCANCODE_A])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        if (keyboardState[SDL_SCANCODE_W])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;

        ByteSwapInplace(state->Gamepad.wButtons);
        ByteSwapInplace(state->Gamepad.sThumbLX);
        ByteSwapInplace(state->Gamepad.sThumbLY);
        ByteSwapInplace(state->Gamepad.sThumbRX);
        ByteSwapInplace(state->Gamepad.sThumbRY);
    }

    void Import_XamInputSetState()
    {
        Log::Stub("XamInputSetState", "Called.");
    }
}

GUEST_FUNCTION_HOOK(__imp__XamInputGetCapabilities, InputHooks::Import_XamInputGetCapabilities)
GUEST_FUNCTION_HOOK(__imp__XamInputGetState, InputHooks::Import_XamInputGetState)
GUEST_FUNCTION_HOOK(__imp__XamInputSetState, InputHooks::Import_XamInputSetState)