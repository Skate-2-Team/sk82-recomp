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

    HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
    {
        DWORD dwShaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        // Enable debugging and skip optimization for easier shader debugging.
        dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

        ID3DBlob *pErrorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                std::cerr << "Error compiling shader (" << szEntryPoint << "): "
                          << (char *)pErrorBlob->GetBufferPointer() << std::endl;
                pErrorBlob->Release();
            }
            return hr;
        }
        else
        {
            Log::Info("ShaderComp", "Shader compiled successfully.");
        }

        if (pErrorBlob)
            pErrorBlob->Release();
        return S_OK;
    }

    HRESULT InitShaders()
    {
        HRESULT hr;
        ID3DBlob *pVSBlob = nullptr;
        hr = CompileShaderFromFile(L"shaders//vp6.fx", "defaultVS", "vs_3_0", &pVSBlob);
        if (FAILED(hr))
        {
            std::cerr << "Failed to compile vertex shader." << std::endl;
            return hr;
        }
        hr = g_video->m_d3dDevice->CreateVertexShader((const DWORD *)pVSBlob->GetBufferPointer(), &g_pVertexShader);
        pVSBlob->Release();
        if (FAILED(hr))
        {
            std::cerr << "Failed to create vertex shader." << std::endl;
            return hr;
        }

        ID3DBlob *pPSBlob = nullptr;
        hr = CompileShaderFromFile(L"shaders//vp6.fx", "defaultPS", "ps_3_0", &pPSBlob);
        if (FAILED(hr))
        {
            std::cerr << "Failed to compile pixel shader." << std::endl;
            return hr;
        }
        hr = g_video->m_d3dDevice->CreatePixelShader((const DWORD *)pPSBlob->GetBufferPointer(), &g_pPixelShader);
        pPSBlob->Release();
        if (FAILED(hr))
        {
            std::cerr << "Failed to create pixel shader." << std::endl;
            return hr;
        }
        return S_OK;
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

        InitShaders();

        // create three dummy textures
        g_video->m_d3dDevice->CreateTexture(1280, 720, 0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &lumTex, nullptr);
        g_video->m_d3dDevice->CreateTexture(640, 360, 0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &blueTex, nullptr);
        g_video->m_d3dDevice->CreateTexture(640, 360, 0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &redTex, nullptr);

        g_isShaderLoaded = true;

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

    HRESULT FillTextureDataRaw(IDirect3DTexture9 *pTexture, UINT srcWidth, UINT srcHeight, const uint8_t *srcData)
    {
        if (!pTexture || !srcData)
            return E_INVALIDARG;

        D3DLOCKED_RECT lockedRect;
        HRESULT hr = pTexture->LockRect(0, &lockedRect, nullptr, 0);
        if (FAILED(hr))
        {
            std::cerr << "Failed to lock texture." << std::endl;
            return hr;
        }

        // If the pitch (bytes per row in the texture) matches srcWidth,
        // we can copy the whole block at once.
        if (lockedRect.Pitch == srcWidth)
        {
            memcpy(lockedRect.pBits, srcData, srcWidth * srcHeight);
        }
        else
        {
            // Otherwise, copy each row individually.
            for (UINT row = 0; row < srcHeight; row++)
            {
                memcpy(reinterpret_cast<uint8_t *>(lockedRect.pBits) + row * lockedRect.Pitch, srcData + row * srcWidth, srcWidth);
            }
        }

        pTexture->UnlockRect(0);
        return S_OK;
    }

    void DumpYUVFrame(const char *filename, uint8_t *yPlane, uint8_t *uPlane, uint8_t *vPlane, int width, int height)
    {
        FILE *f = fopen(filename, "wb");
        if (!f)
        {
            std::cerr << "Failed to open YUV output file!" << std::endl;
            return;
        }

        size_t ySize = width * height;
        size_t uvSize = (width / 2) * (height / 2);

        fwrite(yPlane, 1, ySize, f);
        fwrite(uPlane, 1, uvSize, f);
        fwrite(vPlane, 1, uvSize, f);

        fclose(f);
    }

    PPC_FUNC_IMPL(__imp__sub_8257AA38);
    void VideoRenderer_RwTexture_Render(void *thisPtr, VideoRenderable *videoRenderable, int vp6Border)
    {

        // Log::Info("VideoRenderer_RwTexture_Render", "Rendering frame -> ", videoRenderable->mFrameNumber);

        // convert data vars
        uint8_t *data0 = (uint8_t *)Memory::Translate<uint8_t *>(ByteSwap(videoRenderable->mData[0]));
        uint8_t *data1 = (uint8_t *)Memory::Translate<uint8_t *>(ByteSwap(videoRenderable->mData[1]));
        uint8_t *data2 = (uint8_t *)Memory::Translate<uint8_t *>(ByteSwap(videoRenderable->mData[2]));

        // print addresses

        // Log::Info("VideoRenderer_RwTexture_Render", "Data0: ", (void *)data0, ", Data1: ", (void *)data1, ", Data2: ", (void *)data2);

        int chromaIndex = (((videoRenderable->mWidth + 2) * vp6Border) >> 2);

        uint8_t *lumBuffer = &data0[(videoRenderable->mWidth + 1) * vp6Border];
        uint8_t *uChroma = &data1[chromaIndex];
        uint8_t *vChroma = &data2[chromaIndex];

        // print num buffers used

        HRESULT hrY = FillTextureDataRaw(lumTex, videoRenderable->mWidth, videoRenderable->mHeight, lumBuffer);
        if (FAILED(hrY))
        {
            std::cerr << "Failed to fill luminance texture." << std::endl;
            return;
        }

        // For U and V, use half resolution (assuming 4:2:0 chroma sampling).
        UINT chromaWidth = videoRenderable->mWidth >> 1;   // e.g. 1280/2 = 640
        UINT chromaHeight = videoRenderable->mHeight >> 1; // e.g. 720/2 = 360

        HRESULT hrU = FillTextureDataRaw(blueTex, chromaWidth, chromaHeight, uChroma);
        if (FAILED(hrU))
        {
            std::cerr << "Failed to fill U texture." << std::endl;
            return;
        }

        HRESULT hrV = FillTextureDataRaw(redTex, chromaWidth, chromaHeight, vChroma);
        if (FAILED(hrV))
        {
            std::cerr << "Failed to fill V texture." << std::endl;
            return;
        }

        __imp__sub_8257AA38(*PPCLocal::g_ppcContext, Memory::g_base);
    }

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
            if (primType == XD3DPT_QUADLIST && stride == 24 && g_isShaderLoaded)
            {
                // Log::Info("BeginVerticesBatch", "The viewport is set to: Width: ", viewport.Width, ", Height: ", viewport.Height, ", X: ", viewport.X, ", Y: ", viewport.Y);

                g_video->m_d3dDevice->SetVertexShader(g_pVertexShader);
                g_video->m_d3dDevice->SetPixelShader(g_pPixelShader);

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

                g_video->m_d3dDevice->SetVertexShaderConstantF(0, (float *)&matVp, 4);
                g_video->m_d3dDevice->SetVertexShaderConstantF(4, (float *)&matWorld, 4);

                // Set the texture stage states
                g_video->m_d3dDevice->SetTexture(0, lumTex);
                g_video->m_d3dDevice->SetTexture(1, blueTex);
                g_video->m_d3dDevice->SetTexture(2, redTex);

                struct QuadListVertex
                {
                    be<float> x, y, z, w;
                    be<float> u, v;
                };

                for (int i = 0; i < 3; i++)
                {
                    g_video->m_d3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                    g_video->m_d3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                    g_video->m_d3dDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                }

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
        EnsureSceneActive();

        while (!batchQueue.empty())
        {
            auto currentBatch = batchQueue.front();
            batchQueue.pop();

            currentBatch->Process();

            delete currentBatch;
        }

        EndSceneIfActive();

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

    void D3DDevice_DrawVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 VertexCount, void *vertexData)
    {
    }

    void D3DDevice_DrawIndexedVertices(void *pDevice, XBOXPRIMITIVETYPE PrimitiveType, unsigned __int64 StartIndex, unsigned __int64 IndexCount)
    {
    }

    void D3DDevice_SetVertexShader(void *pDevice, void *pShader)
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

    void D3D_UnlockResource(void *pResource)
    {
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
GUEST_FUNCTION_HOOK(sub_83142500, VideoHooks::D3DDevice_SetSamplerState_MinFilter)
GUEST_FUNCTION_HOOK(sub_831426A8, VideoHooks::D3DDevice_SetSamplerState_MinFilter)
GUEST_FUNCTION_HOOK(sub_831427F0, VideoHooks::D3DDevice_SetSamplerState_MagFilter)
GUEST_FUNCTION_HOOK(sub_83142998, VideoHooks::D3DDevice_SetSamplerState_MagFilter)
GUEST_FUNCTION_HOOK(sub_83142BE8, VideoHooks::D3DDevice_SetSamplerState_SeparateZFilterEnable)
GUEST_FUNCTION_HOOK(sub_83142D30, VideoHooks::D3DDevice_SetSamplerState_MaxAnisotropy)
GUEST_FUNCTION_HOOK(sub_83142E60, VideoHooks::D3DDevice_SetSamplerState_AnisotropyBias)
GUEST_FUNCTION_HOOK(sub_83142FC8, VideoHooks::D3DDevice_SetSamplerState_MipMapLodBias)
GUEST_FUNCTION_HOOK(sub_83143130, VideoHooks::D3DDevice_SetSamplerState_MaxMipLevel)
GUEST_FUNCTION_HOOK(sub_83143268, VideoHooks::D3DDevice_SetSamplerState_MinMipLevel)

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
GUEST_FUNCTION_HOOK(sub_82364E98, VideoHooks::D3DDevice_SetVertexShader)
GUEST_FUNCTION_HOOK(sub_825E5538, VideoHooks::Sk8_AddParam)

// hacks
GUEST_FUNCTION_HOOK(sub_8236C5F8, VideoHooks::RwMemCopy)
GUEST_FUNCTION_HOOK(sub_8257AA38, VideoHooks::VideoRenderer_RwTexture_Render)
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
