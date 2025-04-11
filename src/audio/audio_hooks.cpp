#include "ppc/ppc_recomp_shared.h"
#include "kernel/function.h"
#include "log.h"
#include <cpu/guest_thread.h>
#include <SDL3/SDL.h>
#include "kernel/heap.h"
#include "kernel/memory.h"

#define AUDIO_DRIVER_KEY (uint32_t)('SKATE')

static PPCFunc *g_clientCallback{};
static uint32_t g_clientCallbackParam{};
static std::unique_ptr<std::thread> g_audioThread{};

namespace AudioHooks
{
    // Needs to be properly implemented
    static void AudioThread()
    {
        std::wstring threadName = L"XAudioClientThread";
        SetThreadDescription(GetCurrentThread(), threadName.c_str());

        GuestThreadContext ctx(0);

        while (true)
        {
            // just keep calling the callback
            ctx.ppcContext.r3.u32 = g_clientCallbackParam;
            g_clientCallback(ctx.ppcContext, Memory::g_base);

            // sleep 1 ms chrono
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        Log::Info("AudioThread", "Thread died.");
    }

    static void XAudioRegisterClient(PPCFunc *callback, uint32_t param)
    {
        auto *pClientParam = static_cast<uint32_t *>(g_heap->Alloc(sizeof(param)));
        ByteSwapInplace(param);
        *pClientParam = param;
        g_clientCallbackParam = Memory::MapVirtual(pClientParam);
        g_clientCallback = callback;

        g_audioThread = std::make_unique<std::thread>(AudioThread);
    }

    uint32_t Import_XAudioRegisterRenderDriverClient(be<uint32_t> *callback, be<uint32_t> *driver)
    {
        Log::Info("XAudioRegisterRenderDriverClient", "Called.");

        *driver = AUDIO_DRIVER_KEY;

        XAudioRegisterClient(Memory::FindFunction(*callback), callback[1]);

        return 0;
    }

    uint32_t Import_XAudioUnregisterRenderDriverClient(uint32_t driver)
    {
        Log::Info("XAudioUnregisterRenderDriverClient", "Unregistered.");

        return 0;
    }

    uint32_t Import_XAudioSubmitRenderDriverFrame(uint32_t driver, void *samples)
    {
        return 0;
    }

    int Import_XAudioGetVoiceCategoryVolumeChangeMask(void *driverClient, unsigned int *outMask)
    {
        if (outMask)
            *outMask = ByteSwap(0x3); // simulate both categories (bit 0 and 1) changed

        return 0;
    }

    uint32_t Import_XAudioGetVoiceCategoryVolume(unsigned int categoryIndex, float *outVolume)
    {
        if (outVolume)
            *outVolume = ByteSwap(1.0f);

        return 0;
    }

    uint32_t Import_XAudioGetSpeakerConfig()
    {
        return 0;
    }
}

GUEST_FUNCTION_HOOK(__imp__XAudioGetSpeakerConfig, AudioHooks::Import_XAudioGetSpeakerConfig)
GUEST_FUNCTION_HOOK(__imp__XAudioRegisterRenderDriverClient, AudioHooks::Import_XAudioRegisterRenderDriverClient)
GUEST_FUNCTION_HOOK(__imp__XAudioUnregisterRenderDriverClient, AudioHooks::Import_XAudioUnregisterRenderDriverClient)
GUEST_FUNCTION_HOOK(__imp__XAudioSubmitRenderDriverFrame, AudioHooks::Import_XAudioSubmitRenderDriverFrame)
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolume, AudioHooks::Import_XAudioGetVoiceCategoryVolume)
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolumeChangeMask, AudioHooks::Import_XAudioGetVoiceCategoryVolumeChangeMask)