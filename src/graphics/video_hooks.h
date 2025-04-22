#pragma once

#include <windows.h>
#include <d3dcompiler.h>
#include <queue>
#include <map>
#include <xxhash.h>
#include <filesystem>
#include <fstream>

#include "kernel/function.h"
#include "kernel/heap.h"
#include "kernel/xbox.h"
#include "ppc/ppc_recomp_shared.h"
#include "video.h"
#include "utils/tsqueue.h"
#include "shaders.h"
#include "game/game_structs.h"
#include "d3d_context.h"

namespace VideoHooks
{
    struct TriangleVertex
    {
        float x, y, z, w;
        float u, v;
    };

    struct TriangleListVertexSwapped
    {
        be<float> x, y, z, w;
        be<float> u, v;
    };

    struct QuadListVertex
    {
        be<float> x, y, z, w;
        be<float> u, v;
    };

    struct D3DRectSwapped
    {
        be<int> x1;
        be<int> y1;
        be<int> x2;
        be<int> y2;
    };

    struct VertexBatchInfo
    {
        void *memory;
        size_t size;
        uint32_t primType;
        UINT vertexCount;
        UINT stride;
    };

    struct GuestViewport
    {
        be<uint32_t> x;
        be<uint32_t> y;
        be<uint32_t> width;
        be<uint32_t> height;
        be<float> minZ;
        be<float> maxZ;
    };

    struct __declspec(align(4)) VideoRenderable
    {
        int mState;
        unsigned int mMagicNumber;
        uint32_t mData[3];
        unsigned int mSize[3];
        be<unsigned int> mStride[3];
        be<unsigned int> mWidth;
        be<unsigned int> mHeight;
        int mFormat;
        be<int> mFrameNumber;
        be<unsigned int> mNumBuffersUsed;
        be<int> mUseCount;
        volatile bool mIsReadyToRender;
        int mCBParams[2];
        uint32_t mContext[3];
        bool mDropFrameFlag;
        bool mFlipFlag;
    };

    struct TextureParameters
    {
        be<renderengine::Type> type;
        be<unsigned int> flags;
        be<unsigned int> width;
        be<unsigned int> height;
        be<unsigned int> depth;
        be<unsigned int> mipLevels;
        be<GuestD3D::PixelFormat> format;
        be<unsigned int> address;
    };

    inline void MatrixIdentity(Matrix4x4 *mat)
    {
        // Set all elements to zero first.
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
                mat->m[i][j] = 0.0f;
        }
        // Set the diagonal to one.
        for (int i = 0; i < 4; ++i)
        {
            mat->m[i][i] = 1.0f;
        }
    }

    namespace Batches
    {
        // Batch Base
        enum BatchType
        {
            BatchType_BeginVertices,
            BatchType_SetPixelShader,
            BatchType_SetViewPort,
            BatchType_SetTexture,
            BatchType_Unknown,
        };

        struct BatchInfo
        {
            BatchType m_type;

            BatchInfo(BatchType type) : m_type(type) {};
            virtual ~BatchInfo() = default;

            virtual void Process() = 0;
        };

        // Different types of Batches
        struct BeginVerticesBatch : public BatchInfo
        {
            void *memory = nullptr;
            size_t size = 0;
            uint32_t primType = 0;
            uint32_t vertexCount = 0;
            uint32_t stride = 0;

            BeginVerticesBatch() : BatchInfo(BatchType_BeginVertices) {};
            ~BeginVerticesBatch() override
            {
                if (memory != nullptr)
                {
                    g_heap->Free(memory);
                    memory = nullptr;
                }
            };

            void Process() override;
        };

        struct SetViewPortBatch : public BatchInfo
        {
            D3DVIEWPORT9 viewport{};

            SetViewPortBatch() : BatchInfo(BatchType_SetViewPort) {};
            ~SetViewPortBatch() override = default;

            void Process() override;
        };

        struct SetPixelShaderBatch : public BatchInfo
        {
            uint32_t shaderKey = 0;

            SetPixelShaderBatch() : BatchInfo(BatchType_SetPixelShader) {};
            ~SetPixelShaderBatch() override = default;

            void Process() override;
        };

        struct SetTextureBatch : public BatchInfo
        {
            renderengine::D3DBaseTexture *baseTexture = nullptr;
            uint32_t samplerID = 0;

            SetTextureBatch() : BatchInfo(BatchType_Unknown) {};
            ~SetTextureBatch() override = default;

            void Process() override;
        };

        struct SetShaderConstantBatch : public BatchInfo
        {
            float *constData = 0;
            uint32_t registerID = 0;
            uint32_t vertexCount = 0;
            bool isPixelShader = false;

            SetShaderConstantBatch() : BatchInfo(BatchType_Unknown) {};
            ~SetShaderConstantBatch() override = default;

            void Process() override;
        };
    }

    struct GuestTexture
    {
        IDirect3DTexture9 *texture = nullptr;
        void *texBuffer = nullptr;
        UINT pitch = 0;
        UINT height = 0;
    };

    inline void *globalBuffer = nullptr;
    inline int lastSize = 0;

    inline std::atomic<GuestDevice *> g_guestDevice = nullptr;

    inline std::map<uint32_t, GuestTexture *> g_textureMap;
    inline ThreadSafeQueue<Batches::BatchInfo *> batchQueue;
}