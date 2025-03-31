#pragma once

#include <windows.h>
#include "ppc/ppc_recomp_shared.h"
#include "video.h"
#include "kernel/function.h"
#include "kernel/heap.h"
#include "kernel/xbox.h"

namespace VideoHooks
{
    inline std::chrono::steady_clock::time_point m_frameStart;
    constexpr auto m_targetFrameTime = std::chrono::milliseconds(16); // Approx. 60 FPS

    struct D3DRectSwapped
    {
        be<int> x1;
        be<int> y1;
        be<int> x2;
        be<int> y2;
    };

    HRESULT Direct3D_CreateDevice(
        unsigned int Adapter,
        int DeviceType,
        void *pUnused,
        unsigned int BehaviorFlags,
        void *pPresentationParameters,
        void **ppReturnedDeviceInterface);

    void *D3DDevice_CreateSurface(
        unsigned int Width,
        unsigned int Height,
        _D3DFORMAT D3DFormat,
        _D3DMULTISAMPLE_TYPE MultiSample,
        const _D3DSURFACE_PARAMETERS *pParameters);

    void *D3DDevice_CreateTexture(
        unsigned int Width,
        unsigned int Height,
        unsigned int Depth,
        unsigned int Levels,
        unsigned int Usage,
        signed int D3DFormat,
        unsigned int Pool,
        _D3DRESOURCETYPE D3DType);

    void D3DDevice_Swap(
        void *pDevice,
        void *pFrontBuffer,
        const void *pParameters);
}