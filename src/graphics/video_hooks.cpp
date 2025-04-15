#include "video_hooks.h"

PPC_FUNC(MainLoopHook)
{
    // We need this hook because the SDL polling won't work if its not on main thread.
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

        return S_OK;
    }

    void ConvertQuadListToTriangleList(const QuadListVertex *quadVertices, TriangleVertex *triangleVertices, uint32_t quadCount)
    {
        // XD3DPT_QUADLIST doesn't exist on PC dx9, so we must split the quads into 2 triangles manually.

        for (uint32_t i = 0; i < quadCount; i++)
        {
            const QuadListVertex &v0 = quadVertices[i * 4 + 0];
            const QuadListVertex &v1 = quadVertices[i * 4 + 1];
            const QuadListVertex &v2 = quadVertices[i * 4 + 2];
            const QuadListVertex &v3 = quadVertices[i * 4 + 3];

            // First triangle (v0, v1, v2)
            uint32_t baseIdx = i * 6;
            triangleVertices[baseIdx + 0] = {
                v0.x, v0.y, v0.z, v0.w,
                v0.u, v0.v};

            triangleVertices[baseIdx + 1] = {
                v1.x, v1.y, v1.z, v1.w,
                v1.u, v1.v};

            triangleVertices[baseIdx + 2] = {
                v2.x, v2.y, v2.z, v2.w,
                v2.u, v2.v};

            // Second triangle (v0, v2, v3)
            triangleVertices[baseIdx + 3] = {
                v0.x, v0.y, v0.z, v0.w,
                v0.u, v0.v};

            triangleVertices[baseIdx + 4] = {
                v2.x, v2.y, v2.z, v2.w,
                v2.u, v2.v};

            triangleVertices[baseIdx + 5] = {
                v3.x, v3.y, v3.z, v3.w,
                v3.u, v3.v};
        }
    }

    namespace Batches
    {
        inline uint32_t curPixelShader = 0;

        void SetViewPortBatch::Process()
        {
            g_video->m_d3dDevice->SetViewport(&viewport);
        }

        void BeginVerticesBatch::Process()
        {
            if (!Shaders::g_isShaderLoaded)
                return;

            if (primType == XD3DPT_QUADLIST && stride == 24)
            {
                g_video->m_d3dDevice->SetVertexShader(Shaders::g_pVertexShader);

                uint32_t quadCount = vertexCount / 4;
                uint32_t triangleVertexCount = quadCount * 6;

                // print all of the quad list vertices
                const QuadListVertex *quadVertices = (QuadListVertex *)memory;
                std::vector<TriangleVertex> triangleVertices(triangleVertexCount);

                ConvertQuadListToTriangleList(quadVertices, triangleVertices.data(), quadCount);

                g_video->m_d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
                g_video->m_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, triangleVertexCount / 3, triangleVertices.data(), sizeof(TriangleVertex));
            }
            else if (primType == XD3DPT_TRIANGLELIST && stride == 24)
            {
                auto swappedVertices = (TriangleListVertexSwapped *)(memory);
                std::vector<TriangleVertex> hostVertices(vertexCount);

                ConvertVertices(swappedVertices, hostVertices.data(), vertexCount);

                g_video->m_d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
                g_video->m_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertexCount / 3, hostVertices.data(), sizeof(TriangleVertex));

                // Log::Info("TriangleList", "Requesting shader -> ", Shaders::g_pPixelShaderNames[curShader]);
            }
            else
            {
                // Log::Info("BeginVertices", "PrimType -> ", primType, ", Vertex Count -> ", vertexCount, ", Stride -> ", stride);
            }
        }

        void SetPixelShaderBatch::Process()
        {
            curPixelShader = shaderKey;

            // check if the shader is in the map
            auto it = Shaders::g_pPixelShaders.find(shaderKey);

            if (it != Shaders::g_pPixelShaders.end())
            {
                // Doesn't matter if shader is null, will only be 0 for that draw call.
                g_video->m_d3dDevice->SetPixelShader(it->second);
            }
        }

        void SetTextureBatch::Process()
        {
            // check if text is in map
            auto it = g_textureMap.find(baseTexture->Identifier);

            if (it != g_textureMap.end())
                g_video->m_d3dDevice->SetTexture(samplerID, it->second->texture);
        }

        void SetShaderConstantBatch::Process()
        {
            if (isPixelShader)
                g_video->m_d3dDevice->SetPixelShaderConstantF(registerID, constData, vertexCount);
            else
                g_video->m_d3dDevice->SetVertexShaderConstantF(registerID, constData, vertexCount);
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

        auto batchInfo = new Batches::BeginVerticesBatch();

        batchInfo->size = stride * vertexCount;
        batchInfo->memory = g_heap->AllocPhysical(batchInfo->size, 128);
        batchInfo->primType = PrimitiveType;
        batchInfo->vertexCount = vertexCount;
        batchInfo->stride = stride;

        batchQueue.push(batchInfo);

        return Memory::MapVirtual(batchInfo->memory);
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

    void D3DDevice_DrawVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 VertexCount, void *vertexData)
    {
    }

    void D3DDevice_DrawIndexedVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 StartIndex, unsigned __int64 IndexCount)
    {
    }

    void D3DDevice_SetTexture(void *pDevice, unsigned int Sampler, renderengine::D3DBaseTexture *pTexture)
    {
        auto textureBatch = new Batches::SetTextureBatch();

        textureBatch->baseTexture = pTexture;
        textureBatch->samplerID = Sampler;

        batchQueue.push(textureBatch);
    }

    // Properly implement these
    void D3DDevice_Clear(
        void *pDevice,
        unsigned int Count,
        const D3DRectSwapped *pRects,
        unsigned int Flags,
        void *Color,
        double Z,
        unsigned int Stencil)
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

    PPC_FUNC_IMPL(__imp__sub_829FF6D0);
    uint32_t Texture_Initialize(rw::Resource *resource, const TextureParameters *params)
    {
        __imp__sub_829FF6D0(*PPCLocal::g_ppcContext, Memory::g_base);

        auto texture = Memory::Translate<renderengine::Texture *>(PPCLocal::g_ppcContext->r3.u32);

        auto guestTex = new GuestTexture;

        auto d3dFormat = GetD3DFormat(params->format);

        if (d3dFormat == 0)
        {
            Log::Info("Texture_Initialize", "Invalid format -> ", (unsigned long)params->format, ", Texture -> ", texture->Identifier);
        }

        // Lock and Unlock are broken as of right now
        if (d3dFormat != D3DFMT_L8)
        {
            return PPCLocal::g_ppcContext->r3.u32;
        }

        D3DPOOL pool = D3DPOOL_DEFAULT;
        DWORD usage = D3DUSAGE_DYNAMIC;

        if (d3dFormat == D3DFMT_DXT1 || d3dFormat == D3DFMT_DXT4 || d3dFormat == D3DFMT_DXT5)
        {
            pool = D3DPOOL_MANAGED;
            usage = 0;
        }

        // make a d3d texture from the resource
        HRESULT hr = g_video->m_d3dDevice->CreateTexture(params->width, params->height, params->mipLevels, usage, d3dFormat, pool, &guestTex->texture, nullptr);

        texture->Identifier.set(g_textureMap.size() + 1);

        if (FAILED(hr))
        {
            Log::Error("Texture_Initialize", "Failed to create texture, all params -> ", params->width, ", ", params->height, ", ", params->mipLevels, ", Format -> ", d3dFormat, " Pool -> ", pool, " Usage -> ", usage, " Result -> ", (unsigned long)hr);
        }
        else
        {
            Log::Info("Texture_Initialize", "Created texture -> ", params->width, ", ", params->height, ", ", params->mipLevels, ", ", params->format, " Key: ", texture->Identifier);
        }

        g_textureMap[texture->Identifier] = guestTex;

        return PPCLocal::g_ppcContext->r3.u32;
    }

    PPC_FUNC_IMPL(__imp__sub_823BF740);
    void D3D_LockSurface(renderengine::Texture *pTexture) // arguments arent reliable on guest hook here
    {
        uint64_t AsyncBlock = PPCLocal::g_ppcContext->r6.u64;
        uint32_t *Level = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r8.u32);
        uint32_t *Flags = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r9.u32);

        // check if the texture is in the map
        auto it = g_textureMap.find(pTexture->Identifier);
        if (it == g_textureMap.end() || pTexture->Identifier == 0)
        {
            __imp__sub_823BF740(*PPCLocal::g_ppcContext, Memory::g_base);

            return;
        }

        GuestTexture *guestTex = it->second;

        // Assume mipLevel in AsyncBlock's low 32-bits
        UINT mipLevel = (UINT)(AsyncBlock & 0xFFFFFFFF);

        D3DSURFACE_DESC desc;
        guestTex->texture->GetLevelDesc(mipLevel, &desc);

        if (desc.Format == D3DFMT_DXT1 || desc.Format == D3DFMT_DXT3 || desc.Format == D3DFMT_DXT5)
        {
            Log::Error("D3D_LockSurface", "Trying to lock a compressed texture. ");
            DebugBreak();
        }

        UINT pitch = desc.Width;
        UINT height = desc.Height;
        UINT size = pitch * desc.Height;

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
    void D3D_UnlockResource(renderengine::D3DResource *pResource)
    {
        auto it = g_textureMap.find(pResource->Identifier);
        if (it == g_textureMap.end() || pResource->Identifier == 0)
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
            Log::Error("D3D_UnlockResource", "Failed to lock texture: ", guestTex->texture, " | Result -> ", hr);
            return;
        }

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

    void D3DDevice_KickOff()
    {
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
        if (destPtr == 0 || sourcePtr == 0)
        {
            Log::Error("RwMemCopy", "INVALID MEM COPY: ", destPtr, ", ", sourcePtr);
            return;
        }

        __imp__sub_8236C5F8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

}

GUEST_FUNCTION_HOOK(sub_82A5D368, VideoHooks::Direct3D_CreateDevice)
GUEST_FUNCTION_HOOK(sub_823A7C98, VideoHooks::D3DDevice_Swap)
GUEST_FUNCTION_HOOK(sub_82381BD0, VideoHooks::D3DDevice_Clear)
GUEST_FUNCTION_HOOK(sub_82381AA8, VideoHooks::D3DDevice_ClearF)
GUEST_FUNCTION_HOOK(sub_82352910, VideoHooks::D3DDevice_SetTexture)

GUEST_FUNCTION_HOOK(sub_823615A8, VideoHooks::D3DDevice_BeginVertices)
GUEST_FUNCTION_HOOK(sub_82A61218, VideoHooks::D3DDevice_BeginIndexedVertices)
GUEST_FUNCTION_HOOK(sub_82363AC8, VideoHooks::D3DDevice_DrawVertices)
GUEST_FUNCTION_HOOK(sub_82364240, VideoHooks::D3DDevice_DrawIndexedVertices)

GUEST_FUNCTION_HOOK(sub_823BF740, VideoHooks::D3D_LockSurface)
GUEST_FUNCTION_HOOK(sub_823A6BC0, VideoHooks::D3D_LockResource)
GUEST_FUNCTION_HOOK(sub_82325F50, VideoHooks::D3D_UnlockResource)
GUEST_FUNCTION_HOOK(sub_8235FCE0, VideoHooks::D3D_SetViewport)

GUEST_FUNCTION_HOOK(sub_829FF6D0, VideoHooks::Texture_Initialize)

// hacks
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::RwMemCopy)
GUEST_FUNCTION_HOOK(sub_829EF3B8, VideoHooks::AsyncOp_Open)
GUEST_FUNCTION_HOOK(sub_829CF1F0, VideoHooks::VideoDecoder_Vp6_Decode)

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