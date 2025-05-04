#define NOMINMAX

#include "video_hooks.h"

PPC_FUNC(MainLoopHook)
{
    // We need this hook because the SDL polling won't work if its not on main thread.
    g_video->EventFlush();

    return;
}

namespace VideoHooks
{
    static void PrintCurrentVertexDecl(bool printFromHostDevice)
    {
        LPDIRECT3DVERTEXDECLARATION9 decl9 = nullptr;

        if (printFromHostDevice)
        {
            g_video->m_d3dDevice->GetVertexDeclaration(&decl9);
        }
        else
        {
            // 1) grab the guest’s current decl ID
            uint32_t id = g_guestDevice.load()->m_vertexDeclaration.get();

            // 2) look it up in our cache
            auto it = g_vertexDeclMap.find(id);
            if (it == g_vertexDeclMap.end())
            {
                Log::Info("VideoHooks", "No vertex declaration cached for ID ", id);
                return;
            }

            decl9 = it->second;
        }

        D3DVERTEXELEMENT9 elems[32];
        UINT elemCount = _countof(elems);

        HRESULT hr = decl9->GetDeclaration(elems, &elemCount);
        if (FAILED(hr))
        {
            Log::Error("VideoHooks", "GetDeclaration failed: ", hr);
            return;
        }

        Log::Info("PrintCurrentVertexDecl", "-------------------------------");

        for (UINT i = 0; i < elemCount; ++i)
        {
            const auto &e = elems[i];
            // Stream == 0xFF
            if (e.Stream == 0xFF)
                break;

            Log::Info("VideoHooks",
                      "Elem[", i, "]: Stream=", e.Stream,
                      ", Offset=", e.Offset,
                      ", Type=", GetHostD3DDeclTypeName(e.Type),
                      ", Method=", GuestD3D::GetDeclMethodName(e.Method),
                      ", Usage=", GuestD3D::GetDeclUsageName(e.Usage),
                      ", UsageIdx=", (int)e.UsageIndex);
        }
    }

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

    void PrepareVertexData(void *p_vertexBuffer, int size, int alignment)
    {
        // copy data over in chunks of 4 bytes and bswap
        for (int i = 0; i < size; i += 4)
        {
            uint32_t *src = (uint32_t *)((uint8_t *)p_vertexBuffer + i);
            ByteSwapInplace(*src);
        }
    }

    void *PrepareQuadData(void *p_vertexBuffer, int quadCount, int stride, int allocSize)
    {
        PrepareVertexData(p_vertexBuffer, allocSize, 16);

        int triangleVertexCount = quadCount * 6; // Each quad becomes 2 triangles (6 vertices)

        void *triangleBuffer = g_heap->AllocPhysical(triangleVertexCount * stride, 16);

        // For each quad, create two triangles
        for (int i = 0; i < quadCount; i++)
        {
            uint8_t *srcQuad = static_cast<uint8_t *>(p_vertexBuffer) + (i * 4 * stride);
            uint8_t *dstTri = static_cast<uint8_t *>(triangleBuffer) + (i * 6 * stride);

            // Quad vertices: A, B, C, D (in counter-clockwise order)
            // Triangle 1: A, B, D
            // Triangle 2: B, C, D

            // First triangle (A, B, D)
            memcpy(dstTri, srcQuad, stride);                           // A
            memcpy(dstTri + stride, srcQuad + stride, stride);         // B
            memcpy(dstTri + 2 * stride, srcQuad + 3 * stride, stride); // D

            // Second triangle (B, C, D)
            memcpy(dstTri + 3 * stride, srcQuad + stride, stride);     // B
            memcpy(dstTri + 4 * stride, srcQuad + 2 * stride, stride); // C
            memcpy(dstTri + 5 * stride, srcQuad + 3 * stride, stride); // D
        }

        return triangleBuffer;
    }

    PPC_FUNC_IMPL(__imp__sub_82A62FA8);
    void Guest_XGSetVertexDeclaration(const GuestD3D::VertexElement *p_vertexElements, uint32_t p_vertexDeclaration)
    {
        __imp__sub_82A62FA8(*PPCLocal::g_ppcContext, Memory::g_base);

        // if we've already created this declaration, skip
        auto it = g_vertexDeclMap.find(p_vertexDeclaration);
        if (it != g_vertexDeclMap.end())
        {
            // This shouldn't really happen
            Log::Info("VideoHooks", "Vertex declaration already exists: ", p_vertexDeclaration, " Map Size -> ", g_vertexDeclMap.size());
            DebugBreak();
            return;
        }

        // build D3D9 declaration
        std::vector<D3DVERTEXELEMENT9> d3dDecl;

        for (int i = 0;; ++i)
        {
            const auto &src = p_vertexElements[i];

            // D3DDECL_END() has Stream == 0xFF
            if (src.Stream == 0xFF)
                break;

            D3DVERTEXELEMENT9 elem;
            elem.Stream = src.Stream.get();
            elem.Offset = src.Offset.get();
            elem.Type = ConvertGuestDeclType(src.Type.get());
            elem.Method = src.Method;
            elem.Usage = src.Usage;
            elem.UsageIndex = src.UsageIndex;

            d3dDecl.push_back(elem);
        }

        d3dDecl.push_back(D3DDECL_END());

        LPDIRECT3DVERTEXDECLARATION9 d3dDeclPtr = nullptr;
        HRESULT hr = g_video->m_d3dDevice->CreateVertexDeclaration(d3dDecl.data(), &d3dDeclPtr);

        if (FAILED(hr))
        {
            Log::Error("VideoHooks", "Failed to create vertex declaration: ", hr);
            DebugBreak();
            return;
        }

        // Store the declaration in the map
        g_vertexDeclMap[p_vertexDeclaration] = d3dDeclPtr;

        Log::Info("XGSetVertexDeclaration", "Created vertex declaration: ", p_vertexDeclaration, " Map Size -> ", g_vertexDeclMap.size());
    }

    PPC_FUNC_IMPL(__imp__sub_8239C798);
    void Guest_Sk8VertexProgramStateFactory_GetVertexProgramState(void *p_this, renderengine::VertexDescriptor *p_vd)
    {
        __imp__sub_8239C798(*PPCLocal::g_ppcContext, Memory::g_base);

        // Figure out what vertex decl its asking for
        auto it = g_vertexDeclMap.find(p_vd->m_d3dVertexDeclaration.get());

        if (it == g_vertexDeclMap.end())
        {
            Log::Error("VideoHooks", "Vertex declaration not found: ", p_vd->m_d3dVertexDeclaration.get());
            DebugBreak();
            return;
        }

        LPDIRECT3DVERTEXDECLARATION9 d3dDecl = it->second;
        // Set the vertex declaration on the device
        HRESULT hr = g_video->m_d3dDevice->SetVertexDeclaration(d3dDecl);

        if (FAILED(hr))
        {
            Log::Error("VideoHooks", "Failed to set vertex declaration: ", hr);
            DebugBreak();
            return;
        }

        // set it on the device
        g_guestDevice.load()->m_vertexDeclaration = p_vd->m_d3dVertexDeclaration.get();
    }

    void Guest_Swap()
    {
        g_video->PresentFrame();
        g_video->UpdateFPSCounter();
    }

    uint32_t Guest_BeginVertices(GuestDevice *p_device, GuestD3D::PrimitiveType p_primType, uint32_t p_vertexCount, uint32_t p_stride)
    {
        auto pendingDraw = new PendingDraw();

        pendingDraw->m_type = p_primType;
        pendingDraw->m_vertexCount = p_vertexCount;
        pendingDraw->m_stride = p_stride;
        pendingDraw->m_allocSize = p_vertexCount * p_stride;
        pendingDraw->m_primCount = GetPrimitiveCount(p_vertexCount, p_primType);
        pendingDraw->m_vertexBuffer = g_heap->AllocPhysical(pendingDraw->m_allocSize, 16);

        g_pendingDrawQueue.push(pendingDraw);

        return Memory::MapVirtual(pendingDraw->m_vertexBuffer);
    }

    static void EndVertices()
    {
        while (!g_pendingDrawQueue.empty())
        {
            auto pendingDraw = g_pendingDrawQueue.front();
            g_pendingDrawQueue.pop();

            if (pendingDraw)
            {
                g_video->m_d3dDevice->BeginScene();

                // Log::Info("BeginVertices", "PrimType -> ", PrimitiveType, ", Vertex Count -> ", vertexCount, ", Stride -> ", stride);

                // copy first 8 registers from guest to host (debug)
                for (int i = 0; i < 8; i++)
                {
                    // current register
                    float curRegisterVS[4] =
                        {
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[i].x),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[i].y),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[i].z),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[i].w),
                        };

                    // current register
                    int psIndex = i + 256;
                    float curRegisterPS[4] =
                        {
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[psIndex].v[0]),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[psIndex].v[1]),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[psIndex].v[2]),
                            ByteSwapFloat(g_guestDevice.load()->m_constants.Alu[psIndex].v[3]),
                        };

                    g_video->m_d3dDevice->SetVertexShaderConstantF(i, curRegisterVS, 1);
                    g_video->m_d3dDevice->SetPixelShaderConstantF(i, curRegisterPS, 1);
                }

                if (pendingDraw->m_type == GuestD3D::PrimitiveType::XD3DPT_QUADLIST)
                {
                    int trianglePrimCount = (pendingDraw->m_primCount * 6) / 3;

                    if (Shaders::g_pVertexShader)
                        g_video->m_d3dDevice->SetVertexShader(Shaders::g_pVertexShader);

                    auto quadPreparedData = PrepareQuadData(pendingDraw->m_vertexBuffer, pendingDraw->m_primCount, pendingDraw->m_stride, pendingDraw->m_allocSize);

                    // PrintCurrentVertexDecl(true);

                    // print current shader
                    // Log::Info("EndVertices", "Requesting shader -> ", Shaders::g_pPixelShaderNames[Shaders::g_curShaderKey]);

                    g_video->m_d3dDevice->DrawPrimitiveUP(ConvertGuestPrimType(pendingDraw->m_type), trianglePrimCount, quadPreparedData, pendingDraw->m_stride);
                    g_heap->Free(quadPreparedData);
                }
                else
                {
                    PrepareVertexData(pendingDraw->m_vertexBuffer, pendingDraw->m_allocSize, 16);
                    g_video->m_d3dDevice->DrawPrimitiveUP(ConvertGuestPrimType(pendingDraw->m_type), pendingDraw->m_primCount, pendingDraw->m_vertexBuffer, pendingDraw->m_stride);
                }

                g_video->m_d3dDevice->EndScene();

                // only delete if it wasent null
                delete pendingDraw;
            }
        }
    }

    PPC_FUNC_IMPL(__imp__sub_82366560);
    void Guest_SimpleDraw_Draw()
    {
        __imp__sub_82366560(*PPCLocal::g_ppcContext, Memory::g_base);

        EndVertices();
    }

    PPC_FUNC_IMPL(__imp__sub_8239C4C0);
    void Guest_SimpleDraw_Draw_TriListTextured_Fasttrack()
    {
        __imp__sub_8239C4C0(*PPCLocal::g_ppcContext, Memory::g_base);

        EndVertices();
    }

    PPC_FUNC_IMPL(__imp__sub_826A8A98);
    void Guest_DrawstringLocal_Char()
    {
        __imp__sub_826A8A98(*PPCLocal::g_ppcContext, Memory::g_base);

        EndVertices();
    }

    PPC_FUNC_IMPL(__imp__sub_826A8E10);
    void Guest_DrawstringLocal_UShort()
    {
        __imp__sub_826A8E10(*PPCLocal::g_ppcContext, Memory::g_base);

        EndVertices();
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

    void Guest_SetTexture(GuestDevice *p_device, uint32_t p_sampler, GuestBaseTexture *p_texture)
    {
        auto it = g_textureMap.find(p_texture->Identifier);

        if (it != g_textureMap.end())
        {
            g_curTextureKey = p_texture->Identifier.get();

            g_video->m_d3dDevice->SetTexture(p_sampler, it->second->texture);
        }
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

    static int texCount = 0;

    PPC_FUNC_IMPL(__imp__sub_829FF6D0);
    void Guest_InitTexture(rw::Resource *p_resource, const TextureParameters *p_params)
    {
        __imp__sub_829FF6D0(*PPCLocal::g_ppcContext, Memory::g_base);

        auto texture = Memory::Translate<GuestBaseTexture *>(PPCLocal::g_ppcContext->r3.u32);

        if (p_params->address != 0 && p_params->format == GuestD3D::PixelFormat::PIXELFORMAT_DXT5 && p_params->mipLevels == 1)
        {
            // get texture
            XTexHelper texHelper;

            auto rawTexelData = Memory::Translate<uint8_t *>(p_params->address);

            // base address
            auto constants = ((GuestBaseTexture *)texture)->GetBitField();

            // Log::Info("Guest_InitTexture", "Base address: ", (void *)(Memory::Translate<uint8_t *>(constants.BaseAddress << 12)));
            Log::Info("Guest_InitTexture", "Params address: ", (void *)rawTexelData);

            // dump into vec
            // std::vector<uint8_t> data(rawTexelData, rawTexelData + p_params->width * p_params->height * 4);

            Log::Info("Guest_InitTexture", "Flags -> ", (uint32_t)p_params->flags,
                      " Width -> ", (uint32_t)p_params->width,
                      " Height -> ", (uint32_t)p_params->height,
                      " Depth -> ", (uint32_t)p_params->depth,
                      " MipLevels -> ", (uint32_t)p_params->mipLevels,
                      " Format -> ", (uint32_t)p_params->format,
                      " Address -> ", (uint32_t)p_params->address);

            Log::Info("Guest_InitTexture", "Texture Dimension -> ", (uint32_t)constants.Dimension);

            if (constants.Dimension == 3)
            {
                Log::Info("Guest_InitTexture", "Texture is a cube map.");
                DebugBreak();
                return;
            }

            // 16384

            auto textureData = texHelper.GetTextureData((GuestBaseTexture *)texture, rawTexelData);

            auto d3d9Tex = texHelper.ConvertToDXTexture(g_video->m_d3dDevice, textureData);

            /*
            std::string texName = "tex_dump_" + std::to_string(texCount) + ".bin";

            // dump tex to file
            std::ofstream outFile(texName, std::ios::binary | std::ios::trunc);

            outFile.write(reinterpret_cast<const char *>(rawTexelData), textureData.slicePitch);
            outFile.close();*/

            auto convertGuestTex = new GuestTexture;

            convertGuestTex->texture = d3d9Tex;
            convertGuestTex->pitch = textureData.rowPitch;
            convertGuestTex->height = textureData.height;

            texture->Identifier.set(g_textureMap.size() + 1);

            Log::Info("Guest_InitTexture", "Converted texture -> ", textureData.width,
                      " height: ", textureData.height,
                      " blockWidth: ", textureData.widthInBlocks,
                      " blockHeight: ", textureData.heightInBlocks,
                      " rowPitch: ", textureData.rowPitch,
                      " slicePitch: ", textureData.slicePitch, " Key: ", texture->Identifier);

            g_textureMap[texture->Identifier] = convertGuestTex;

            // texHelper.WriteToDDS(textureData, std::to_string(texture->Identifier) + "_converted.dds");

            texCount++;

            return;
        }

        auto guestTex = new GuestTexture;
        auto d3dFormat = GetD3DFormat(p_params->format);

        if (d3dFormat == 0)
            Log::Info("Texture_Initialize", "Invalid format -> ", (unsigned long)p_params->format, ", Texture -> ", texture->Identifier);

        // Lock and Unlock are broken as of right now
        if (d3dFormat != D3DFMT_L8)
            return;

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
    }

    PPC_FUNC_IMPL(__imp__sub_823BF740);
    void Guest_LockSurface(GuestBaseTexture *p_texture) // arguments arent reliable on guest hook here
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

        if (desc.Format == D3DFMT_DXT5)
        {
            DebugBreak();
        }

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
    void Guest_UnlockResource(GuestResource *p_resource)
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
        const D3DVIEWPORT9 viewport = {DWORD(p_x), DWORD(p_y), DWORD(p_width), DWORD(p_height), float(p_minZ), float(p_maxZ)};

        g_video->m_d3dDevice->SetViewport(&viewport);
    }

    void D3DDevice_KickOff()
    {
    }

    // Start of State Setting
    void Guest_SetRasterizerState(renderengine::RasterizerState *p_state)
    {
        g_video->m_d3dDevice->SetRenderState(D3DRS_CULLMODE, GetCullMode(p_state->m_cullMode));
        g_video->m_d3dDevice->SetRenderState(D3DRS_FILLMODE, GetFillMode(p_state->m_fillMode));
        g_video->m_d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, p_state->m_scissorTestEnable);
        g_video->m_d3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, p_state->m_slopeScaleDepthBias);
        g_video->m_d3dDevice->SetRenderState(D3DRS_DEPTHBIAS, p_state->m_depthBias);
        g_video->m_d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, p_state->m_multiSampleAntiAlias);
        g_video->m_d3dDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, p_state->m_multiSampleMask);

        // g_video->m_d3dDevice->SetRenderState(D3DRS_VIEWPORTENABLE, p_state->m_viewportEnable); //will need to be emulated
        // g_video->m_d3dDevice->SetRenderState(D3DRS_HALFPIXELOFFSET, p_state->m_halfPixelOffset); // will need to be emulated
        // g_video->m_d3dDevice->SetRenderState(D3DRS_PRIMITIVETESTENABLE, p_state->m_primitiveTestEnable); // will need to be emulated

        if (p_state->m_primitiveResetEnable)
        {
            // g_video->m_d3dDevice->SetRenderState(D3DRS_PRIMITIVERESETINDEX, p_state->m_primitiveResetIndex); // will need to be emulated
        }
    }

    void Guest_SetSamplerState(renderengine::SamplerState *p_state, uint32_t p_sampler)
    {
        auto samplerPack = p_state->GetSwappedPack();

        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_ADDRESSU, GetTexAddress(samplerPack.m_addressU));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_ADDRESSV, GetTexAddress(samplerPack.m_addressV));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_ADDRESSW, GetTexAddress(samplerPack.m_addressW));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_BORDERCOLOR, samplerPack.m_borderColor);

        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MIPFILTER, GetTexFilter(samplerPack.m_mipfilter));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MINFILTER, GetTexFilter(samplerPack.m_minfilter));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MAGFILTER, GetTexFilter(samplerPack.m_magfilter));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MAXANISOTROPY, samplerPack.m_maxanisotropy);
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MIPMAPLODBIAS, static_cast<DWORD>(p_state->m_mipmaplodBias));
        g_video->m_d3dDevice->SetSamplerState(p_sampler, D3DSAMP_MAXMIPLEVEL, samplerPack.m_maxmiplevel);

        // missing a few cases in here.
        // clamp policy, hgradientexpbias, vgradientexpbias, trilinear threshold, forcebcwtomax
    }

    void Guest_SetBlendState(renderengine::BlendStatePack *p_state)
    {
        // byte swap first blend state
        GuestD3D::BlendState state{};
        state.m_dword = ByteSwap(p_state->m_blendStates[0].m_dword);

        // we dont have per render target blending in d3d9
        // parse first state
        g_video->m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, GetBlend(state.m_srcBlend));
        g_video->m_d3dDevice->SetRenderState(D3DRS_BLENDOP, GetBlendOp(state.m_blendOp));
        g_video->m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, GetBlend(state.m_destBlend));

        g_video->m_d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, GetBlend(state.m_srcBlendAlpha));
        g_video->m_d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, GetBlendOp(state.m_blendOpAlpha));
        g_video->m_d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, GetBlend(state.m_destBlendAlpha));

        g_video->m_d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, p_state->m_colorWriteEnable);
        g_video->m_d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE1, p_state->m_colorWriteEnable1);
        g_video->m_d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE2, p_state->m_colorWriteEnable2);
        g_video->m_d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE3, p_state->m_colorWriteEnable3);
        g_video->m_d3dDevice->SetRenderState(D3DRS_BLENDFACTOR, p_state->m_blendFactor);
        g_video->m_d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, p_state->m_alphaTestEnable);
        g_video->m_d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, GetCmpFunc(p_state->m_alphaFunc));
        g_video->m_d3dDevice->SetRenderState(D3DRS_ALPHAREF, p_state->m_alphaRef);
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
GUEST_FUNCTION_HOOK(sub_82A62FA8, VideoHooks::Guest_XGSetVertexDeclaration)
GUEST_FUNCTION_HOOK(sub_8239C798, VideoHooks::Guest_Sk8VertexProgramStateFactory_GetVertexProgramState)

// Texture related functions
GUEST_FUNCTION_HOOK(sub_82352910, VideoHooks::Guest_SetTexture)
GUEST_FUNCTION_HOOK(sub_829FF6D0, VideoHooks::Guest_InitTexture)
GUEST_FUNCTION_HOOK(sub_823A6BC0, VideoHooks::Guest_LockResource)
GUEST_FUNCTION_HOOK(sub_82325F50, VideoHooks::Guest_UnlockResource)
GUEST_FUNCTION_HOOK(sub_823BF740, VideoHooks::Guest_LockSurface)

// State related functions
GUEST_FUNCTION_HOOK(sub_8235D5C0, VideoHooks::Guest_SetRasterizerState)
GUEST_FUNCTION_HOOK(sub_82365518, VideoHooks::Guest_SetSamplerState)
GUEST_FUNCTION_HOOK(sub_82360DB8, VideoHooks::Guest_SetBlendState)

// Game specific hacks
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::Guest_RwMemCopy)
GUEST_FUNCTION_HOOK(sub_829EF3B8, VideoHooks::Guest_AsyncOp_Open)
GUEST_FUNCTION_HOOK(sub_829CF1F0, VideoHooks::Guest_VideoDecoder_Vp6_Decode)

GUEST_FUNCTION_HOOK(sub_82366560, VideoHooks::Guest_SimpleDraw_Draw)
GUEST_FUNCTION_HOOK(sub_8239C4C0, VideoHooks::Guest_SimpleDraw_Draw_TriListTextured_Fasttrack)
GUEST_FUNCTION_HOOK(sub_826A8A98, VideoHooks::Guest_DrawstringLocal_Char)
GUEST_FUNCTION_HOOK(sub_826A8E10, VideoHooks::Guest_DrawstringLocal_UShort)

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