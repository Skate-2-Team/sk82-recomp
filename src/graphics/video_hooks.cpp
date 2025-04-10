#include "video_hooks.h"

PPC_FUNC(MainLoopHook)
{
    // We need this hook because the SDL polling won't work if its not
    // on main thread.
    g_video->WindowLoop();

    return;
}

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

        Shaders::PrecompileShaders();

        // to stop crashing
        *ppReturnedDeviceInterface = (void *)Memory::MapVirtual(g_heap->AllocPhysical(22272, 128));

        return ERROR_SUCCESS;
    }

    PPC_FUNC_IMPL(__imp__sub_825E5538);
    void Sk8_AddParam(
        uint8_t *thisPtr,
        uint8_t *type,
        char *pParmName,
        void *DataPtr,
        char uCount)
    {
        Log::Info("Sk8_AddParam", "Parm Name -> ", pParmName, ", Data Ptr -> ", DataPtr);

        if (std::string(pParmName) == "g_matVP")
        {
            g_matVP = (Matrix4x4Swap *)DataPtr;
        }

        __imp__sub_825E5538(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    static bool texReady = false;

    // all test code at the moment
    namespace Batches
    {
        void SetViewPortBatch::Process()
        {
            g_video->m_d3dDevice->SetViewport(&viewport);
        }

        void BeginVerticesBatch::Process()
        {
            // process the batch
            if (primType == XD3DPT_QUADLIST && stride == 24 && Shaders::g_isShaderLoaded)
            {
                // Log::Info("BeginVerticesBatch", "The viewport is set to: Width: ", viewport.Width, ", Height: ", viewport.Height, ", X: ", viewport.X, ", Y: ", viewport.Y);

                g_video->m_d3dDevice->SetVertexShader(Shaders::g_pVertexShader);
                // g_video->m_d3dDevice->SetPixelShader(g_pPixelShader);

                // copy current g_matVP into normal matrix
                Matrix4x4 matVp = {0};
                matVp.m[0][0] = g_matVP->m[0][0].get();
                matVp.m[0][1] = g_matVP->m[0][1].get();
                matVp.m[0][2] = g_matVP->m[0][2].get();
                matVp.m[0][3] = g_matVP->m[0][3].get();
                matVp.m[1][0] = g_matVP->m[1][0].get();
                matVp.m[1][1] = g_matVP->m[1][1].get();
                matVp.m[1][2] = g_matVP->m[1][2].get();
                matVp.m[1][3] = g_matVP->m[1][3].get();
                matVp.m[2][0] = g_matVP->m[2][0].get();
                matVp.m[2][1] = g_matVP->m[2][1].get();
                matVp.m[2][2] = g_matVP->m[2][2].get();
                matVp.m[2][3] = g_matVP->m[2][3].get();
                matVp.m[3][0] = g_matVP->m[3][0].get();
                matVp.m[3][1] = g_matVP->m[3][1].get();
                matVp.m[3][2] = g_matVP->m[3][2].get();
                matVp.m[3][3] = g_matVP->m[3][3].get();

                Matrix4x4 matWorld;
                MatrixIdentity(&matWorld);

                // c4 - g_matVP
                // c5 - i_matWorld

                g_video->m_d3dDevice->SetVertexShaderConstantF(4, (float *)&matVp, 4);
                g_video->m_d3dDevice->SetVertexShaderConstantF(8, (float *)&matWorld, 4);

                g_video->m_d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

                // Set the texture stage states
                if (texReady)
                {
                    auto it = g_textureMap.begin();

                    g_video->m_d3dDevice->SetTexture(0, it->second->texture);
                    it++;

                    g_video->m_d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                    g_video->m_d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

                    g_video->m_d3dDevice->SetTexture(1, it->second->texture);
                    it++;

                    g_video->m_d3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                    g_video->m_d3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

                    g_video->m_d3dDevice->SetTexture(2, it->second->texture);

                    g_video->m_d3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                    g_video->m_d3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

                    g_textureMap.end();
                }

                struct QuadListVertex
                {
                    be<float> x, y, z, w;
                    be<float> u, v;
                };

                // print all of the quad list vertices
                QuadListVertex *quadVertices = (QuadListVertex *)memory;

                struct TriangleVertex
                {
                    float x, y, z, w;
                    float u, v;
                };

                uint32_t quadCount = vertexCount / 4;

                const uint32_t triangleVertexCount = quadCount * 6;
                std::vector<TriangleVertex> triangleVertices(triangleVertexCount);

                for (uint32_t i = 0; i < quadCount; i++)
                {
                    const QuadListVertex &v0 = quadVertices[i * 4 + 0];
                    const QuadListVertex &v1 = quadVertices[i * 4 + 1];
                    const QuadListVertex &v2 = quadVertices[i * 4 + 2];
                    const QuadListVertex &v3 = quadVertices[i * 4 + 3];

                    triangleVertices[i * 6 + 0] = {v0.x.get(), v0.y.get(), v0.z.get(), v0.w.get(), v0.u.get(), v0.v.get()};
                    triangleVertices[i * 6 + 1] = {v1.x.get(), v1.y.get(), v1.z.get(), v1.w.get(), v1.u.get(), v1.v.get()};
                    triangleVertices[i * 6 + 2] = {v2.x.get(), v2.y.get(), v2.z.get(), v2.w.get(), v2.u.get(), v2.v.get()};

                    triangleVertices[i * 6 + 3] = {v0.x.get(), v0.y.get(), v0.z.get(), v0.w.get(), v0.u.get(), v0.v.get()};
                    triangleVertices[i * 6 + 4] = {v2.x.get(), v2.y.get(), v2.z.get(), v2.w.get(), v2.u.get(), v2.v.get()};
                    triangleVertices[i * 6 + 5] = {v3.x.get(), v3.y.get(), v3.z.get(), v3.w.get(), v3.u.get(), v3.v.get()};
                }

                g_video->m_d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

                // render a fullscreen quad

                g_video->m_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, triangleVertexCount / 3, triangleVertices.data(), sizeof(TriangleVertex));
            }
        }
    }

    void D3DDevice_Swap()
    {
        g_video->m_d3dDevice->BeginScene();

        while (!batchQueue.empty())
        {
            auto currentBatch = batchQueue.front();
            batchQueue.pop();

            currentBatch->Process();

            delete currentBatch;
        }

        g_video->m_d3dDevice->EndScene();

        g_video->m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
    }

    PPC_FUNC_IMPL(__imp__sub_829CF1F0);
    BOOL VideoDecoder_Vp6_Decode(
        uint8_t *thisPtr,
        char *buffer,
        unsigned int bufferSize,
        int frameNumber,
        void *videoRenderable)
    {

        Log::Info("VideoDecoder_Vp6_Decode", "Processing frame -> ", frameNumber);

        __imp__sub_829CF1F0(*PPCLocal::g_ppcContext, Memory::g_base);

        return PPCLocal::g_ppcContext->r3.u32;
    }

    uint32_t D3DDevice_BeginVertices(void *pDevice, uint32_t PrimitiveType, unsigned __int64 vertexCount, uint32_t stride)
    {
        // Log::Info("BeginVertices", "PrimType -> ", PrimitiveType, ", Vertex Count -> ", vertexCount, ", Stride -> ", stride);
        size_t totalSize = stride * vertexCount;

        auto batchInfo = new Batches::BeginVerticesBatch();

        batchInfo->memory = g_heap->AllocPhysical(totalSize, 128);
        batchInfo->size = totalSize;
        batchInfo->primType = PrimitiveType;
        batchInfo->vertexCount = vertexCount;
        batchInfo->stride = stride;

        batchQueue.push(batchInfo);

        return Memory::MapVirtual(batchInfo->memory);
    }

    static bool hasPrinted = false;

    PPC_FUNC_IMPL(__imp__sub_82BCEEA8);
    void cFxParser_StartElement(
        void *thisPtr,
        char *name)
    {
        if (!hasPrinted)
        {
            // Log::Info("cFxParser_StartElement", "Name -> ", name);

            hasPrinted = true;
        }

        __imp__sub_82BCEEA8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    void D3DDevice_DrawVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 VertexCount, void *vertexData)
    {
    }

    void D3DDevice_DrawIndexedVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 StartIndex, unsigned __int64 IndexCount)
    {
    }

    void D3DDevice_KickOff()
    {
    }

    void D3DDevice_SetTexture(void *pDevice, unsigned int Sampler, void *pTexture)
    {
        // Log::Info("D3DDevice_SetTexture", "Setting sampler -> ", Sampler, ", Texture -> ", pTexture);

        if (Sampler == 2)
        {
        }
    }

    HRESULT D3DDevice_BeginIndexedVertices(
        void *pDevice,
        XBOXPRIMITIVETYPE PrimitiveType,
        unsigned __int64 NumVertices,
        unsigned __int64 IndexDataFormat,
        unsigned int VertexStreamZeroStride,
        void **ppIndexData,
        void **ppVertexData)
    {
        return S_OK;
    }

    // Properly implement these
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
        // g_video->m_d3dDevice->Clear(Count, (_D3DRECT *)pRects, Flags, 0xFF000000, Z, Stencil);
    }

    void D3DDevice_ClearF(
        void *pDevice,
        unsigned int Flags,
        unsigned __int64 pColor,
        double Z,
        void *Stencil)
    {
        // g_video->m_d3dDevice->Clear(0, nullptr, Flags, 0xFF000000, Z, 0);
    }

    PPC_FUNC_IMPL(__imp__sub_829EF3B8);
    void AsyncOp_Open(
        void *thisPtr,
        const char *FilePath,
        unsigned int modeflags,
        void *doneCallback,
        void *context,
        volatile int priority)
    {
        Log::Info("AsyncOp_Open", "File Path -> ", FilePath);

        __imp__sub_829EF3B8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    // This needs to be fixed, something is passing a null ptr into it, at destPtr.
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

    PPC_FUNC_IMPL(__imp__sub_829FF6D0);
    uint32_t Texture_Initialize(uint32_t resource, const TextureParameters *params)
    {
        __imp__sub_829FF6D0(*PPCLocal::g_ppcContext, Memory::g_base);

        // currently only store if the type is PIXELFORMAT_LIN_L8

        if (params->format == PIXELFORMAT_LIN_L8)
        {
            // make a d3d texture from the resource
            IDirect3DTexture9 *pTexture = nullptr;
            HRESULT hr = g_video->m_d3dDevice->CreateTexture(params->width, params->height, params->mipLevels, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &pTexture, nullptr);

            if (FAILED(hr))
            {
                std::cerr << "Failed to create texture." << std::endl;
            }

            auto guestTex = new GuestTexture;

            guestTex->texture = pTexture;

            g_textureMap[PPCLocal::g_ppcContext->r3.u32] = guestTex;

            Log::Info("Texture_Initialize", "Make a texture -> ", params->width, ", ", params->height, ", ", params->mipLevels, ", ", params->format);

            if (g_textureMap.size() == 3)
            {
                texReady = true;
            }
        }

        return PPCLocal::g_ppcContext->r3.u32;
    }

    PPC_FUNC_IMPL(__imp__sub_823BF740);
    void D3D_LockSurface(uint32_t pTexture) // arguments arent reliable on guest hook here
    {
        uint64_t AsyncBlock = PPCLocal::g_ppcContext->r6.u64;
        uint32_t *Level = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r8.u32);
        uint32_t *Flags = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r9.u32);

        // check if the texture is in the map
        auto it = g_textureMap.find(pTexture);
        if (it == g_textureMap.end())
        {
            __imp__sub_823BF740(*PPCLocal::g_ppcContext, Memory::g_base);

            return;
        }

        GuestTexture *guestTex = it->second;

        // Assume mipLevel in AsyncBlock's low 32-bits
        UINT mipLevel = (UINT)(AsyncBlock & 0xFFFFFFFF);

        D3DSURFACE_DESC desc;
        guestTex->texture->GetLevelDesc(mipLevel, &desc);

        UINT pitch = desc.Width; // assuming L8 format
        UINT height = desc.Height;
        UINT size = pitch * desc.Height;

        /// stride

        guestTex->pitch = pitch;
        guestTex->height = height;

        if (guestTex->texBuffer)
        {
            g_heap->Free(guestTex->texBuffer);
        }

        guestTex->texBuffer = g_heap->AllocPhysical(size, 128);

        *Level = ByteSwap(Memory::MapVirtual(guestTex->texBuffer));
        *Flags = ByteSwap(pitch);
    }

    PPC_FUNC_IMPL(__imp__sub_82325F50);
    void D3D_UnlockResource(uint32_t pResource)
    {
        // Log::Info("D3D_UnlockResource", "Unlocking resource: ", pResource);

        auto it = g_textureMap.find(pResource);
        if (it == g_textureMap.end())
        {
            __imp__sub_82325F50(*PPCLocal::g_ppcContext, Memory::g_base);
            return;
        }

        GuestTexture *guestTex = it->second;

        // unlock it and write the data to the texture
        D3DLOCKED_RECT lockedRect;
        HRESULT hr = guestTex->texture->LockRect(0, &lockedRect, nullptr, 0);

        if (FAILED(hr))
        {
            std::cerr << "Failed to lock texture." << std::endl;
            return;
        }

        // lock tex mutex

        for (UINT row = 0; row < guestTex->height; row++)
        {
            memcpy(reinterpret_cast<uint8_t *>(lockedRect.pBits) + row * lockedRect.Pitch, reinterpret_cast<uint8_t *>(guestTex->texBuffer) + row * guestTex->pitch, guestTex->pitch);
        }

        // Unlock the texture
        guestTex->texture->UnlockRect(0);

        g_heap->Free(guestTex->texBuffer);
        guestTex->texBuffer = nullptr;
        guestTex->pitch = 0;
        guestTex->height = 0;
    }

    PPC_FUNC_IMPL(__imp__sub_827A2D78);
    int FileLoader_SyncLoad(
        void *thisPtr,
        char *filename)
    {
        // Log::Info("FileLoader_SyncLoad", "Loading file: ", filename);

        __imp__sub_827A2D78(*PPCLocal::g_ppcContext, Memory::g_base);

        if (std::string(filename).contains("vp6_"))
        {
        }

        return PPCLocal::g_ppcContext->r3.u32;
    }

    // This is stop mem leaks just for now
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
        if (lastSize < Size)
        {
            if (globalBuffer != nullptr)
            {
                g_heap->Free(globalBuffer);
            }

            globalBuffer = g_heap->AllocPhysical(Size, 128);
            lastSize = Size;
        }

        return Memory::MapVirtual(globalBuffer);
    }

    void D3D_SetViewport(
        void *pDevice,
        double X,
        double Y,
        double Width,
        double Height,
        double MinZ,
        double MaxZ,
        unsigned int Flags)
    {
        auto batchInfo = new Batches::SetViewPortBatch();

        batchInfo->viewport = {DWORD(X), DWORD(Y), DWORD(Width), DWORD(Height), float(MinZ), float(MaxZ)};

        batchQueue.push(batchInfo);
    }

    // sampler states
    void D3DDevice_SetSamplerState_MinFilter(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
        g_video->m_d3dDevice->SetSamplerState(Sampler, D3DSAMP_MINFILTER, Value);
    }

    void D3DDevice_SetSamplerState_MagFilter(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
        g_video->m_d3dDevice->SetSamplerState(Sampler, D3DSAMP_MAGFILTER, Value);
    }

    void D3DDevice_SetSamplerState_SeparateZFilterEnable(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
    }

    void D3DDevice_SetSamplerState_MaxAnisotropy(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
        g_video->m_d3dDevice->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, Value);
    }

    void D3DDevice_SetSamplerState_AnisotropyBias(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
    }

    void D3DDevice_SetSamplerState_MipMapLodBias(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
    }

    void D3DDevice_SetSamplerState_MaxMipLevel(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
        g_video->m_d3dDevice->SetSamplerState(Sampler, D3DSAMP_MAXMIPLEVEL, Value);
    }

    void D3DDevice_SetSamplerState_MinMipLevel(void *pDevice, unsigned int Sampler, unsigned int Value)
    {
    }
}

GUEST_FUNCTION_HOOK(sub_82352910, VideoHooks::D3DDevice_SetTexture)

// samplers
/*
GUEST_FUNCTION_HOOK(sub_83142500, VideoHooks::D3DDevice_SetSamplerState_MinFilter)
GUEST_FUNCTION_HOOK(sub_831426A8, VideoHooks::D3DDevice_SetSamplerState_MinFilter)
GUEST_FUNCTION_HOOK(sub_831427F0, VideoHooks::D3DDevice_SetSamplerState_MagFilter)
GUEST_FUNCTION_HOOK(sub_83142998, VideoHooks::D3DDevice_SetSamplerState_MagFilter)
GUEST_FUNCTION_HOOK(sub_83142BE8, VideoHooks::D3DDevice_SetSamplerState_SeparateZFilterEnable)
GUEST_FUNCTION_HOOK(sub_83142D30, VideoHooks::D3DDevice_SetSamplerState_MaxAnisotropy)
GUEST_FUNCTION_HOOK(sub_83142E60, VideoHooks::D3DDevice_SetSamplerState_AnisotropyBias)
GUEST_FUNCTION_HOOK(sub_83142FC8, VideoHooks::D3DDevice_SetSamplerState_MipMapLodBias)
GUEST_FUNCTION_HOOK(sub_83143130, VideoHooks::D3DDevice_SetSamplerState_MaxMipLevel)
GUEST_FUNCTION_HOOK(sub_83143268, VideoHooks::D3DDevice_SetSamplerState_MinMipLevel)*/

GUEST_FUNCTION_HOOK(sub_82A5D368, VideoHooks::Direct3D_CreateDevice)
GUEST_FUNCTION_HOOK(sub_823A7C98, VideoHooks::D3DDevice_Swap)
GUEST_FUNCTION_HOOK(sub_82381BD0, VideoHooks::D3DDevice_Clear)
GUEST_FUNCTION_HOOK(sub_82381AA8, VideoHooks::D3DDevice_ClearF)

GUEST_FUNCTION_HOOK(sub_823615A8, VideoHooks::D3DDevice_BeginVertices)
GUEST_FUNCTION_HOOK(sub_82A61218, VideoHooks::D3DDevice_BeginIndexedVertices)
GUEST_FUNCTION_HOOK(sub_82363AC8, VideoHooks::D3DDevice_DrawVertices)
GUEST_FUNCTION_HOOK(sub_82364240, VideoHooks::D3DDevice_DrawIndexedVertices)

GUEST_FUNCTION_HOOK(sub_823A6BC0, VideoHooks::D3D_LockResource)
GUEST_FUNCTION_HOOK(sub_82325F50, VideoHooks::D3D_UnlockResource)
GUEST_FUNCTION_HOOK(sub_8235FCE0, VideoHooks::D3D_SetViewport)

GUEST_FUNCTION_HOOK(sub_829FF6D0, VideoHooks::Texture_Initialize)
GUEST_FUNCTION_HOOK(sub_823BF740, VideoHooks::D3D_LockSurface)
GUEST_FUNCTION_HOOK(sub_825E5538, VideoHooks::Sk8_AddParam)
GUEST_FUNCTION_HOOK(sub_82BCEEA8, VideoHooks::cFxParser_StartElement)

// hacks
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::RwMemCopy)
GUEST_FUNCTION_HOOK(sub_829EF3B8, VideoHooks::AsyncOp_Open)
GUEST_FUNCTION_HOOK(sub_829CF1F0, VideoHooks::VideoDecoder_Vp6_Decode)
GUEST_FUNCTION_HOOK(sub_827A2D78, VideoHooks::FileLoader_SyncLoad)

GUEST_FUNCTION_STUB(sub_8233F578) // D3DDevice_SetShaderGPRAllocation
GUEST_FUNCTION_STUB(sub_8235FBE8) // D3DDevice_SetScissorRect
GUEST_FUNCTION_STUB(sub_823622E8) // D3DDevice_Resolve
GUEST_FUNCTION_STUB(sub_823A5878) // D3DDevice_SetPredication
GUEST_FUNCTION_STUB(sub_82360650) // D3DDevice_SetSurfaces
GUEST_FUNCTION_STUB(sub_8239C960) // D3DDevice_BeginTiling
GUEST_FUNCTION_STUB(sub_823A5398) // D3DDevice_EndTiling
GUEST_FUNCTION_STUB(sub_8232D3D8) // D3D::SynchronizeToPresentationInterval

GUEST_FUNCTION_STUB(sub_8235F8F8) // D3D::SetSurfaceClip
GUEST_FUNCTION_STUB(sub_82363EC0) // D3D::SetPending_RenderStates
GUEST_FUNCTION_STUB(sub_82364740) // D3D::SetPending_AluConstants
GUEST_FUNCTION_STUB(sub_823630F0) // D3D::SetLiteralShaderConstants