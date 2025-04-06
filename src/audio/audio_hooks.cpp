#include "ppc/ppc_recomp_shared.h"
#include "kernel/function.h"
#include "log.h"

#define AUDIO_DRIVER_KEY (uint32_t)('DAUD')

namespace AudioHooks
{
    uint32_t Import_XAudioRegisterRenderDriverClient(be<uint32_t> *callback, be<uint32_t> *driver)
    {
        Log::Info("XAudioRegisterRenderDriverClient", "Called.");

        *driver = AUDIO_DRIVER_KEY;

        return 0;
    }

    uint32_t Import_XAudioUnregisterRenderDriverClient(uint32_t driver)
    {
        Log::Info("XAudioUnregisterRenderDriverClient", "Unregistered.");

        return 0;
    }

    uint32_t Import_XAudioSubmitRenderDriverFrame(uint32_t driver)
    {
        Log::Info("XAudioSubmitRenderDriverFrame", "Called.");

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