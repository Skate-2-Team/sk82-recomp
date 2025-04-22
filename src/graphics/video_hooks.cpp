#include "video_hooks.h"

PPC_FUNC(MainLoopHook)
{
    // We need this hook because the SDL polling won't work if its not on main thread.
    g_video->WindowLoop();

    return;
}

namespace VideoHooks
{
    HRESULT Guest_CreateDevice(int, D3DDEVTYPE p_deviceType, int, uint32_t p_behaviourFlags, void *p_presentationParams, be<uint32_t> *p_returnedDevice)
    {
        Log::Info("VideoHooks", "Direct3D_CreateDevice has been called by game.");

        g_video->MakeWindow();

        if (!g_video->InitD3D())
        {
            Log::Error("VideoHooks", "Failed to initialize D3D.");
            DebugBreak();
            return E_FAIL;
        }

        if (!g_guestDevice)
        {
            g_guestDevice = g_heap->AllocPhysical<GuestDevice>();
            *p_returnedDevice = Memory::MapVirtual(g_guestDevice);
        }

        Shaders::PrecompileShaders();

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

            if (primType == GuestD3D::PrimitiveType::XD3DPT_QUADLIST && stride == 24)
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
            else if (primType == GuestD3D::PrimitiveType::XD3DPT_TRIANGLELIST && stride == 24)
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

    void Guest_Swap()
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

    uint32_t Guest_BeginVertices(GuestDevice *p_device, uint32_t p_primType, uint32_t p_vertexCount, uint32_t p_stride)
    {
        // Log::Info("BeginVertices", "PrimType -> ", PrimitiveType, ", Vertex Count -> ", vertexCount, ", Stride -> ", stride);

        if (p_device->m_vertexDeclaration != 0)
        {
            auto vertexDecl = Memory::Translate<GuestVertexDeclaration *>(p_device->m_vertexDeclaration);
            vertexDecl->PrintDeclaration();
        }

        auto batchInfo = new Batches::BeginVerticesBatch();

        batchInfo->size = p_stride * p_vertexCount;
        batchInfo->memory = g_heap->AllocPhysical(batchInfo->size, 128);
        batchInfo->primType = p_primType;
        batchInfo->vertexCount = p_vertexCount;
        batchInfo->stride = p_stride;

        batchQueue.push(batchInfo);

        return Memory::MapVirtual(batchInfo->memory);
    }

    uint32_t Guest_BeginIndexedVertices(
        GuestDevice *p_device,
        GuestD3D::PrimitiveType p_primType,
        uint64_t p_numVertices,
        uint64_t p_indexDataFormat,
        uint32_t p_vertexZeroStride,
        be<uint32_t> *p_ppIndexData,
        be<uint32_t> *p_ppVertexData)
    {
        return S_OK;
    }

    void Guest_DrawVertices(GuestDevice *p_device, GuestD3D::PrimitiveType p_primType, uint64_t p_vertexCount, void *p_vertexData)
    {
    }

    void Guest_DrawIndexedVertices(GuestDevice *p_device, GuestD3D::PrimitiveType p_primType, uint64_t p_startIndex, uint64_t p_indexCount)
    {
    }

    void Guest_SetTexture(GuestDevice *p_device, uint32_t p_sampler, renderengine::D3DBaseTexture *p_texture)
    {
        auto textureBatch = new Batches::SetTextureBatch();

        textureBatch->baseTexture = p_texture;
        textureBatch->samplerID = p_sampler;

        batchQueue.push(textureBatch);
    }

    void Guest_Clear(
        GuestDevice *p_device,
        uint32_t p_count,
        const D3DRectSwapped *p_rects,
        uint32_t p_flags,
        uint32_t p_color,
        double p_z,
        uint32_t p_stencil)
    {
        // g_video->m_d3dDevice->Clear(Count, (_D3DRECT *)pRects, Flags, 0xFF000000, Z, Stencil);
    }

    void Guest_ClearF(
        void *pDevice,
        uint32_t p_flags,
        void *p_color,
        double p_z,
        uint32_t p_stencil)
    {
        // g_video->m_d3dDevice->Clear(0, nullptr, Flags, 0xFF000000, Z, 0);
    }

    PPC_FUNC_IMPL(__imp__sub_829FF6D0);
    uint32_t Guest_InitTexture(rw::Resource *p_resource, const TextureParameters *p_params)
    {
        __imp__sub_829FF6D0(*PPCLocal::g_ppcContext, Memory::g_base);

        auto texture = Memory::Translate<renderengine::Texture *>(PPCLocal::g_ppcContext->r3.u32);

        auto guestTex = new GuestTexture;

        auto d3dFormat = GetD3DFormat(p_params->format);

        if (d3dFormat == 0)
        {
            Log::Info("Texture_Initialize", "Invalid format -> ", (unsigned long)p_params->format, ", Texture -> ", texture->Identifier);
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
        HRESULT hr = g_video->m_d3dDevice->CreateTexture(p_params->width, p_params->height, p_params->mipLevels, usage, d3dFormat, pool, &guestTex->texture, nullptr);

        texture->Identifier.set(g_textureMap.size() + 1);

        if (FAILED(hr))
        {
            Log::Error("Texture_Initialize", "Failed to create texture, all params -> ", p_params->width, ", ", p_params->height, ", ", p_params->mipLevels, ", Format -> ", d3dFormat, " Pool -> ", pool, " Usage -> ", usage, " Result -> ", (unsigned long)hr);
        }
        else
        {
            Log::Info("Texture_Initialize", "Created texture -> ", p_params->width, ", ", p_params->height, ", ", p_params->mipLevels, ", ", p_params->format, " Key: ", texture->Identifier);
        }

        g_textureMap[texture->Identifier] = guestTex;

        return PPCLocal::g_ppcContext->r3.u32;
    }

    PPC_FUNC_IMPL(__imp__sub_823BF740);
    void Guest_LockSurface(renderengine::Texture *p_texture) // arguments arent reliable on guest hook here
    {
        uint64_t AsyncBlock = PPCLocal::g_ppcContext->r6.u64;
        uint32_t *Level = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r8.u32);
        uint32_t *Flags = Memory::Translate<uint32_t *>(PPCLocal::g_ppcContext->r9.u32);

        // check if the texture is in the map
        auto it = g_textureMap.find(p_texture->Identifier);
        if (it == g_textureMap.end() || p_texture->Identifier == 0)
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
    void Guest_UnlockResource(renderengine::D3DResource *p_resource)
    {
        auto it = g_textureMap.find(p_resource->Identifier);
        if (it == g_textureMap.end() || p_resource->Identifier == 0)
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
    uint32_t Guest_LockResource(void *p_resource,
                                uint64_t p_asyncBlock,
                                DWORD p_blockType,
                                uint32_t p_level,
                                uint32_t p_base,
                                uint32_t p_mip,
                                uint32_t p_data,
                                uint32_t p_size)
    {
        if (lastSize < p_size)
        {
            if (globalBuffer != nullptr)
            {
                g_heap->Free(globalBuffer);
            }

            globalBuffer = g_heap->AllocPhysical(p_size, 128);
            lastSize = p_size;
        }

        return Memory::MapVirtual(globalBuffer);
    }

    void Guest_SetViewport(
        GuestDevice *p_device,
        double p_x,
        double p_y,
        double p_width,
        double p_height,
        double p_minZ,
        double p_maxZ,
        uint32_t p_flags)
    {
        auto batchInfo = new Batches::SetViewPortBatch();

        batchInfo->viewport = {DWORD(p_x), DWORD(p_y), DWORD(p_width), DWORD(p_height), float(p_minZ), float(p_maxZ)};

        batchQueue.push(batchInfo);
    }

    void D3DDevice_KickOff()
    {
    }

    PPC_FUNC_IMPL(__imp__sub_829EF3B8);
    void Guest_AsyncOp_Open(void *, const char *p_filePath)
    {
        Log::Info("AsyncOp_Open", "File Path -> ", p_filePath);

        __imp__sub_829EF3B8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    // This needs to be fixed, something is passing a null ptr into it, at destPtr.
    PPC_FUNC_IMPL(__imp__sub_8236C5F8);
    void Guest_RwMemCopy(uint32_t p_destPtr, uint32_t p_sourcePtr)
    {
        if (p_destPtr == 0 || p_sourcePtr == 0)
        {
            Log::Error("RwMemCopy", "INVALID MEM COPY: ", p_destPtr, ", ", p_sourcePtr);
            return;
        }

        __imp__sub_8236C5F8(*PPCLocal::g_ppcContext, Memory::g_base);
    }

    PPC_FUNC_IMPL(__imp__sub_829CF1F0);
    uint32_t Guest_VideoDecoder_Vp6_Decode(void *, char *, uint32_t, int p_frameNumber)
    {
        // Log::Info("VideoDecoder_Vp6_Decode", "Processing frame -> ", p_frameNumber);

        __imp__sub_829CF1F0(*PPCLocal::g_ppcContext, Memory::g_base);

        return PPCLocal::g_ppcContext->r3.u32;
    }
}

GUEST_FUNCTION_HOOK(sub_82A5D368, VideoHooks::Guest_CreateDevice)
GUEST_FUNCTION_HOOK(sub_823A7C98, VideoHooks::Guest_Swap)

GUEST_FUNCTION_HOOK(sub_82381BD0, VideoHooks::Guest_Clear)
GUEST_FUNCTION_HOOK(sub_82381AA8, VideoHooks::Guest_ClearF)
GUEST_FUNCTION_HOOK(sub_8235FCE0, VideoHooks::Guest_SetViewport)

// Drawing related functions
GUEST_FUNCTION_HOOK(sub_823615A8, VideoHooks::Guest_BeginVertices)
GUEST_FUNCTION_HOOK(sub_82363AC8, VideoHooks::Guest_DrawVertices)

GUEST_FUNCTION_HOOK(sub_82A61218, VideoHooks::Guest_BeginIndexedVertices)
GUEST_FUNCTION_HOOK(sub_82364240, VideoHooks::Guest_DrawIndexedVertices)

// Texture related functions
GUEST_FUNCTION_HOOK(sub_82352910, VideoHooks::Guest_SetTexture)
GUEST_FUNCTION_HOOK(sub_829FF6D0, VideoHooks::Guest_InitTexture)
GUEST_FUNCTION_HOOK(sub_823A6BC0, VideoHooks::Guest_LockResource)
GUEST_FUNCTION_HOOK(sub_82325F50, VideoHooks::Guest_UnlockResource)
GUEST_FUNCTION_HOOK(sub_823BF740, VideoHooks::Guest_LockSurface)

// Game specific hacks
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::Guest_RwMemCopy)
GUEST_FUNCTION_HOOK(sub_829EF3B8, VideoHooks::Guest_AsyncOp_Open)
GUEST_FUNCTION_HOOK(sub_829CF1F0, VideoHooks::Guest_VideoDecoder_Vp6_Decode)

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