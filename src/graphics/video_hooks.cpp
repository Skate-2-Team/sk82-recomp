#include "video_hooks.h"

namespace VideoHooks
{
    static std::queue<VertexBatchInfo> batchQueue;
    std::mutex queueMutex;

    static bool g_sceneActive = false;

    void EnsureSceneActive()
    {
        if (!g_sceneActive && g_video->m_d3dDevice)
        {
            HRESULT hr = g_video->m_d3dDevice->BeginScene();
            if (SUCCEEDED(hr))
            {
                g_sceneActive = true;
            }
        }
    }

    void EndSceneIfActive()
    {
        if (g_sceneActive && g_video->m_d3dDevice)
        {
            HRESULT hr = g_video->m_d3dDevice->EndScene();
            if (SUCCEEDED(hr))
            {
                g_sceneActive = false;
            }
        }
    }

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

    void ShowPixelBuffer(void *pPixelBuffer)
    {
        // Log::Info("ShowPixelBuffer", "Stride is -> ", g_currentBatch.stride, " Vertex Count -> ", g_currentBatch.vertexCount, " Prim Type -> ", g_currentBatch.primType);

        std::lock_guard<std::mutex> lock(queueMutex);
        {
            while (!batchQueue.empty())
            {
                VertexBatchInfo currentBatch = batchQueue.front();
                batchQueue.pop();

                // process the batch

                g_heap->Free(currentBatch.memory);
            }
        }

        EndSceneIfActive();

        g_video->m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // roughly 60 FPS until I figure out how to measure frame time

        EnsureSceneActive();
    }

    uint32_t D3DDevice_BeginVertices(
        void *pDevice,
        uint32_t PrimitiveType,
        unsigned __int64 vertexCount,
        uint32_t stride)
    {
        Log::Info("VideoHooks", "BeginVertices: PrimType -> ", PrimitiveType, ", Vertex Count -> ", vertexCount, ", Stride -> ", stride);

        size_t totalSize = stride * vertexCount;

        EnsureSceneActive();

        void *memory = g_heap->AllocPhysical(totalSize, 128);

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            batchQueue.push({memory, totalSize, PrimitiveType, (uint32_t)vertexCount, stride});
        }

        return Memory::MapVirtual(memory);
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

    PPC_FUNC_IMPL(__imp__sub_8236C5F8);
    void RwMemCopy(uint32_t destPtr, uint32_t sourcePtr)
    {
        // Log::Info("RwMemCopy", "Called: ", destPtr, ", ", sourcePtr);

        if (destPtr == 0 || sourcePtr == 0)
        {
            Log::Error("RwMemCopy", "INVALID MEM COPY: ", destPtr, ", ", sourcePtr);
            return;
        }

        __imp__sub_8236C5F8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    void D3DDevice_KickOff()
    {
    }

    uint32_t D3D_LockResource(void *pResource,
                              unsigned __int64 AsyncBlock,
                              _D3DBLOCKTYPE BlockType,
                              unsigned int Level,
                              unsigned int pBase,
                              unsigned int pMip,
                              unsigned int pData,
                              unsigned int Size,
                              unsigned int Flags,
                              unsigned int CoherStatus)
    {
        // Return some memory for the resource

        return Memory::MapVirtual(g_heap->AllocPhysical(Size, 128));
    }

    void __fastcall D3D_SetViewport(
        void *pDevice,
        double X,
        double Y,
        double Width,
        double Height,
        double MinZ,
        double MaxZ,
        unsigned int Flags)
    {
        DWORD xDword = DWORD(X);
        DWORD yDword = DWORD(Y);
        DWORD widthDword = DWORD(Width);
        DWORD heightDword = DWORD(Height);

        const D3DVIEWPORT9 viewport = {xDword, yDword, widthDword, heightDword, float(MinZ), float(MaxZ)};

        // Log::Info("D3D_SetViewport", "Viewport has been set -> ", xDword, ", ", yDword, ", ", widthDword, ", ", heightDword, ", ", float(MinZ), ", ", float(MaxZ));

        g_video->m_d3dDevice->SetViewport(&viewport);
    }
}

GUEST_FUNCTION_HOOK(sub_82A5D368, VideoHooks::Direct3D_CreateDevice)
GUEST_FUNCTION_HOOK(sub_8232CFD0, VideoHooks::ShowPixelBuffer)
GUEST_FUNCTION_HOOK(sub_82381BD0, VideoHooks::D3DDevice_Clear)
GUEST_FUNCTION_HOOK(sub_82381AA8, VideoHooks::D3DDevice_ClearF)
GUEST_FUNCTION_HOOK(sub_823615A8, VideoHooks::D3DDevice_BeginVertices)
GUEST_FUNCTION_HOOK(sub_823A6BC0, VideoHooks::D3D_LockResource)
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::RwMemCopy)
GUEST_FUNCTION_HOOK(sub_8235FCE0, VideoHooks::D3D_SetViewport)

GUEST_FUNCTION_STUB(sub_8233F578) // D3DDevice_SetShaderGPRAllocation
GUEST_FUNCTION_STUB(sub_8235FBE8) // D3DDevice_SetScissorRect
GUEST_FUNCTION_STUB(sub_823661D0) // renderengine::PixelBuffer::Xbox2ResolveTo
GUEST_FUNCTION_STUB(sub_823650A0) // renderengine::postfx::PfxHelper::RenderQuad
GUEST_FUNCTION_STUB(sub_8235B0F8) // renderengine::postfx::PfxHelper::RenderQuad_Transform
GUEST_FUNCTION_STUB(sub_8235F8F8) // D3D::SetSurfaceClip
GUEST_FUNCTION_STUB(sub_82363EC0) // D3D::SetPending_RenderStates
GUEST_FUNCTION_STUB(sub_82358B40) // renderengine::postfx::RenderTarget::Resolve
GUEST_FUNCTION_STUB(sub_82360650) // D3DDevice_SetSurfaces
GUEST_FUNCTION_STUB(sub_82364740) // D3D::SetPending_AluConstants
GUEST_FUNCTION_STUB(sub_823630F0) // D3D::SetLiteralShaderConstants
GUEST_FUNCTION_STUB(sub_82363AC8) // D3DDevice_DrawVertices
GUEST_FUNCTION_STUB(sub_8239C960) // D3DDevice_BeginTiling
GUEST_FUNCTION_STUB(sub_823A5398) // D3DDevice_EndTiling
GUEST_FUNCTION_STUB(sub_82364240) // D3DDevice_DrawIndexedVertices