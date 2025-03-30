#include "hooks.h"

namespace Hooks
{
    void Import_XGetVideoMode(_XVIDEO_MODE *xvideoMode)
    {
        // struct is already byte swapped
        xvideoMode->dwDisplayWidth = 1280;
        xvideoMode->dwDisplayHeight = 720;
        xvideoMode->fIsInterlaced = 0;
        xvideoMode->fIsWideScreen = 0;
        xvideoMode->fIsHiDef = 1;
        xvideoMode->RefreshRate = 60.0f;
        xvideoMode->VideoStandard = 0;
    }

    void Import_VdEnableDisableClockGating()
    {
        Log::Stub("VdEnableDisableClockGating", "Called.");
    }

    void Import_VdPersistDisplay()
    {
        Log::Stub("VdPersistDisplay", "Called.");
    }

    void Import_VdSwap()
    {
        Log::Stub("VdSwap", "Called.");
    }

    void Import_VdGetSystemCommandBuffer()
    {
        Log::Stub("VdGetSystemCommandBuffer", "Called.");
    }

    void Import_VdEnableRingBufferRPtrWriteBack()
    {
        Log::Stub("VdEnableRingBufferRPtrWriteBack", "Called.");
    }

    void Import_VdInitializeRingBuffer()
    {
        Log::Stub("VdInitializeRingBuffer", "Called.");
    }

    void Import_VdSetSystemCommandBufferGpuIdentifierAddress()
    {
        Log::Stub("VdSetSystemCommandBufferGpuIdentifierAddress", "Called.");
    }

    void Import_VdGetCurrentDisplayGamma()
    {
        Log::Stub("VdGetCurrentDisplayGamma", "Called.");
    }

    void Import_VdQueryVideoFlags()
    {
        Log::Stub("VdQueryVideoFlags", "Called.");
    }

    void Import_VdCallGraphicsNotificationRoutines()
    {
        Log::Stub("VdCallGraphicsNotificationRoutines", "Called.");
    }

    void Import_VdQueryVideoMode()
    {
        Log::Stub("VdQueryVideoMode", "Called.");
    }

    void Import_VdInitializeScalerCommandBuffer()
    {
        Log::Stub("VdInitializeScalerCommandBuffer", "Called.");
    }

    void Import_VdRetrainEDRAM()
    {
        Log::Stub("VdRetrainEDRAM", "Called.");
    }

    void Import_VdRetrainEDRAMWorker()
    {
        Log::Stub("VdRetrainEDRAMWorker", "Called.");
    }

    void Import_VdShutdownEngines()
    {
        Log::Stub("VdShutdownEngines", "Called.");
    }

    void Import_VdGetCurrentDisplayInformation()
    {
        Log::Stub("VdGetCurrentDisplayInformation", "Called.");
    }

    void Import_VdSetDisplayMode()
    {
        Log::Stub("VdSetDisplayMode", "Called.");
    }

    void Import_VdSetGraphicsInterruptCallback()
    {
        Log::Stub("VdSetGraphicsInterruptCallback", "Called.");
    }

    void Import_VdInitializeEngines()
    {
        Log::Stub("VdInitializeEngines", "Called.");
    }

    void Import_VdIsHSIOTrainingSucceeded()
    {
        Log::Stub("VdIsHSIOTrainingSucceeded", "Called.");
    }
}

GUEST_FUNCTION_HOOK(__imp__VdEnableDisableClockGating, Hooks::Import_VdEnableDisableClockGating)
GUEST_FUNCTION_HOOK(__imp__VdPersistDisplay, Hooks::Import_VdPersistDisplay)
GUEST_FUNCTION_HOOK(__imp__VdGetSystemCommandBuffer, Hooks::Import_VdGetSystemCommandBuffer)
GUEST_FUNCTION_HOOK(__imp__VdEnableRingBufferRPtrWriteBack, Hooks::Import_VdEnableRingBufferRPtrWriteBack)
GUEST_FUNCTION_HOOK(__imp__VdInitializeRingBuffer, Hooks::Import_VdInitializeRingBuffer)
GUEST_FUNCTION_HOOK(__imp__VdSetSystemCommandBufferGpuIdentifierAddress, Hooks::Import_VdSetSystemCommandBufferGpuIdentifierAddress)
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayGamma, Hooks::Import_VdGetCurrentDisplayGamma)
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoFlags, Hooks::Import_VdQueryVideoFlags)
GUEST_FUNCTION_HOOK(__imp__VdCallGraphicsNotificationRoutines, Hooks::Import_VdCallGraphicsNotificationRoutines)
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoMode, Hooks::Import_VdQueryVideoMode)
GUEST_FUNCTION_HOOK(__imp__VdInitializeScalerCommandBuffer, Hooks::Import_VdInitializeScalerCommandBuffer)
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAM, Hooks::Import_VdRetrainEDRAM)
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAMWorker, Hooks::Import_VdRetrainEDRAMWorker)
GUEST_FUNCTION_HOOK(__imp__VdShutdownEngines, Hooks::Import_VdShutdownEngines)
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayInformation, Hooks::Import_VdGetCurrentDisplayInformation)
GUEST_FUNCTION_HOOK(__imp__VdSetDisplayMode, Hooks::Import_VdSetDisplayMode)
GUEST_FUNCTION_HOOK(__imp__VdIsHSIOTrainingSucceeded, Hooks::Import_VdIsHSIOTrainingSucceeded)
GUEST_FUNCTION_HOOK(__imp__VdInitializeEngines, Hooks::Import_VdInitializeEngines)
GUEST_FUNCTION_HOOK(__imp__VdSetGraphicsInterruptCallback, Hooks::Import_VdSetGraphicsInterruptCallback)
GUEST_FUNCTION_HOOK(__imp__VdSwap, Hooks::Import_VdSwap)

GUEST_FUNCTION_HOOK(__imp__XGetVideoMode, Hooks::Import_XGetVideoMode)
