#pragma once

#include <windows.h>
#include <d3dcompiler.h>
#include <queue>

#include "kernel/function.h"
#include "kernel/heap.h"
#include "kernel/xbox.h"
#include "ppc/ppc_recomp_shared.h"
#include "video.h"
#include "xex.h"

#include "utils/tsqueue.h"

namespace VideoHooks
{
    struct Matrix4x4
    {
        float m[4][4];
    };

    struct Matrix4x4Swap
    {
        be<float> m[4][4];
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
        unsigned int mStride[3];
        be<unsigned int> mWidth;
        be<unsigned int> mHeight;
        int mFormat;
        be<int> mFrameNumber;
        unsigned int mNumBuffersUsed;
        be<int> mUseCount;
        volatile bool mIsReadyToRender;
        int mCBParams[2];
        uint32_t mContext[3];
        bool mDropFrameFlag;
        bool mFlipFlag;
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
            BatchType_SetViewPort,
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
    }

    inline IDirect3DVertexShader9 *g_pVertexShader = nullptr;
    inline IDirect3DPixelShader9 *g_pPixelShader = nullptr;
    inline std::atomic<bool> g_isShaderLoaded = false;

    inline Matrix4x4Swap *g_matVP = nullptr;

    inline IDirect3DTexture9 *lumTex = nullptr;
    inline IDirect3DTexture9 *blueTex = nullptr;
    inline IDirect3DTexture9 *redTex = nullptr;

    inline void *globalBuffer = nullptr;
    inline int lastSize = 0;

    inline bool g_sceneActive = false;

    inline ThreadSafeQueue<Batches::BatchInfo *> batchQueue;
    inline std::mutex queueMutex;
}