#include "video_hooks.h"

namespace VideoHooks
{
    HRESULT Direct3D_CreateDevice(
        unsigned int Adapter,
        int DeviceType,
        void *pUnused,
        unsigned int BehaviorFlags,
        void *pPresentationParameters,
        void **ppReturnedDeviceInterface)
    {
        Log::Info("VideoHooks", "Direct3D_CreateDevice has been called by game.");

        g_video->MakeWindow();

        if (!g_video->InitD3D())
        {
            Log::Error("VideoHooks", "Failed to initialize D3D.");
            DebugBreak();
            return E_FAIL;
        }

        // to stop crashing
        *ppReturnedDeviceInterface = g_heap->AllocPhysical(22272, 128);

        return 1;
    }

    void *D3DDevice_CreateSurface(unsigned int Width, unsigned int Height, _D3DFORMAT D3DFormat, _D3DMULTISAMPLE_TYPE MultiSample, const _D3DSURFACE_PARAMETERS *pParameters)
    {
        Log::Info("VideoHooks", "D3DDevice_CreateSurface has been called -> ", Width, "x", Height, ", Format: 0x", D3DFormat);

        IDirect3DSurface9 *surface = nullptr;

        HRESULT hr = g_video->m_d3dDevice->CreateRenderTarget(
            Width,
            Height,
            D3DFMT_A8R8G8B8,
            MultiSample,
            0,
            FALSE, // Lockable
            &surface,
            nullptr);

        if (FAILED(hr))
        {
            std::cout << "[Error][VideoHooks]: CreateRenderTarget failed: 0x" << std::hex << hr << std::endl;
            return nullptr;
        }

        return surface;
    }

    void *D3DDevice_CreateTexture(unsigned int Width, unsigned int Height, unsigned int Depth, unsigned int Levels, unsigned int Usage, signed int D3DFormat, unsigned int Pool, _D3DRESOURCETYPE D3DType)
    {
        Log::Info("VideoHooks", "D3DDevice_CreateTexture called: ", Width, "x", Height, ", Format: 0x", D3DFormat);

        IDirect3DTexture9 *texture = nullptr;
        HRESULT hr = g_video->m_d3dDevice->CreateTexture(
            Width,
            Height,
            Levels,
            Usage,
            (D3DFORMAT)D3DFMT_A8R8G8B8,
            (D3DPOOL)Pool,
            &texture,
            nullptr);

        if (FAILED(hr))
        {
            std::cout << "[Error][VideoHooks]: CreateTexture failed: 0x" << std::hex << hr << std::endl;
            return nullptr;
        }

        return texture;
    }

    void ShowPixelBuffer(void *pPixelBuffer)
    {
        g_video->WindowLoop();

        g_video->m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        /*
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - m_frameStart);

        if (frameDuration < m_targetFrameTime)
        {
            Log::Info("VideoHooks", "Sleeping for -> ", (m_targetFrameTime - frameDuration).count(), "ms");

        }*/

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Sleep to prevent busy waiting

        Log::Info("ShowPixelBuffer", "Presented.");
    }

    void D3DDevice_Clear(
        void *pDevice,
        unsigned int Count,
        const D3DRectSwapped *pRects,
        unsigned int Flags,
        void *Color,
        double Z,
        unsigned int Stencil,
        int EDRAMClear)
    {
        g_video->m_d3dDevice->Clear(Count, (_D3DRECT *)pRects, Flags, 0xFF000000, Z, Stencil);
    }

    void D3DDevice_ClearF(
        void *pDevice,
        unsigned int Flags,
        unsigned __int64 pColor,
        double Z,
        void *Stencil)
    {
        g_video->m_d3dDevice->Clear(0, nullptr, Flags, 0xFF000000, Z, 0);
    }

    void *D3DDevice_BeginVertices(
        void *pDevice,
        void *PrimitiveType,
        unsigned __int64 VertexStreamZeroStride)
    {
        Log::Info("VideoHooks", "D3DDevice_BeginVertices called: ", PrimitiveType, ", Stride: ", VertexStreamZeroStride);

        // return a bit of memory for the vertices

        // void *pVertices = g_heap->AllocPhysical(0x10000, 128); // 64k of memory for the vertices

        return nullptr;
    }
}

GUEST_FUNCTION_HOOK(sub_82A5D368, VideoHooks::Direct3D_CreateDevice)
GUEST_FUNCTION_HOOK(sub_8232CFD0, VideoHooks::ShowPixelBuffer)
GUEST_FUNCTION_HOOK(sub_82381BD0, VideoHooks::D3DDevice_Clear)
GUEST_FUNCTION_HOOK(sub_82381AA8, VideoHooks::D3DDevice_ClearF)
GUEST_FUNCTION_HOOK(sub_823615A8, VideoHooks::D3DDevice_CreateTexture)
// GUEST_FUNCTION_HOOK(sub_8210E428, VideoHooks::D3DDevice_CreateSurface)
