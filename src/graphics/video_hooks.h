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

    enum PixelFormat : uint32_t
    {
        PIXELFORMAT_DXT1 = 0x1A200152,
        PIXELFORMAT_LIN_DXT1 = 0x1A200052,
        PIXELFORMAT_DXT2 = 0x1A200153,
        PIXELFORMAT_LIN_DXT2 = 0x1A200053,
        PIXELFORMAT_DXT3 = 0x1A200153,
        PIXELFORMAT_LIN_DXT3 = 0x1A200053,
        PIXELFORMAT_DXT4 = 0x1A200154,
        PIXELFORMAT_LIN_DXT4 = 0x1A200054,
        PIXELFORMAT_DXT5 = 0x1A200154,
        PIXELFORMAT_LIN_DXT5 = 0x1A200054,
        PIXELFORMAT_DXN = 0x1A200171,
        PIXELFORMAT_LIN_DXN = 0x1A200071,
        PIXELFORMAT_A8 = 0x4900102,
        PIXELFORMAT_LIN_A8 = 0x4900002,
        PIXELFORMAT_L8 = 0x28000102,
        PIXELFORMAT_LIN_L8 = 0x28000002,
        PIXELFORMAT_R5G6B5 = 0x28280144,
        PIXELFORMAT_LIN_R5G6B5 = 0x28280044,
        PIXELFORMAT_R6G5B5 = 0x28280145,
        PIXELFORMAT_LIN_R6G5B5 = 0x28280045,
        PIXELFORMAT_L6V5U5 = 0x2A200B45,
        PIXELFORMAT_LIN_L6V5U5 = 0x2A200A45,
        PIXELFORMAT_X1R5G5B5 = 0x28280143,
        PIXELFORMAT_LIN_X1R5G5B5 = 0x28280043,
        PIXELFORMAT_A1R5G5B5 = 0x18280143,
        PIXELFORMAT_LIN_A1R5G5B5 = 0x18280043,
        PIXELFORMAT_A4R4G4B4 = 0x1828014F,
        PIXELFORMAT_LIN_A4R4G4B4 = 0x1828004F,
        PIXELFORMAT_X4R4G4B4 = 0x2828014F,
        PIXELFORMAT_LIN_X4R4G4B4 = 0x2828004F,
        PIXELFORMAT_Q4W4V4U4 = 0x1A20AB4F,
        PIXELFORMAT_LIN_Q4W4V4U4 = 0x1A20AA4F,
        PIXELFORMAT_A8L8 = 0x800014A,
        PIXELFORMAT_LIN_A8L8 = 0x800004A,
        PIXELFORMAT_G8R8 = 0x2D20014A,
        PIXELFORMAT_LIN_G8R8 = 0x2D20004A,
        PIXELFORMAT_V8U8 = 0x2D20AB4A,
        PIXELFORMAT_LIN_V8U8 = 0x2D20AA4A,
        PIXELFORMAT_D16 = 0x1A220158,
        PIXELFORMAT_LIN_D16 = 0x1A220058,
        PIXELFORMAT_L16 = 0x28000158,
        PIXELFORMAT_LIN_L16 = 0x28000058,
        PIXELFORMAT_R16F = 0x2DA2AB5E,
        PIXELFORMAT_LIN_R16F = 0x2DA2AA5E,
        PIXELFORMAT_R16F_EXPAND = 0x2DA2AB5B,
        PIXELFORMAT_LIN_R16F_EXPAND = 0x2DA2AA5B,
        PIXELFORMAT_UYVY = 0x1A20014C,
        PIXELFORMAT_LIN_UYVY = 0x1A20004C,
        PIXELFORMAT_LE_UYVY = 0x1A20010C,
        PIXELFORMAT_LE_LIN_UYVY = 0x1A20000C,
        PIXELFORMAT_G8R8_G8B8 = 0x1828014C,
        PIXELFORMAT_LIN_G8R8_G8B8 = 0x1828004C,
        PIXELFORMAT_R8G8_B8G8 = 0x1828014B,
        PIXELFORMAT_LIN_R8G8_B8G8 = 0x1828004B,
        PIXELFORMAT_YUY2 = 0x1A20014B,
        PIXELFORMAT_LIN_YUY2 = 0x1A20004B,
        PIXELFORMAT_LE_YUY2 = 0x1A20010B,
        PIXELFORMAT_LE_LIN_YUY2 = 0x1A20000B,
        PIXELFORMAT_A8R8G8B8 = 0x18280186,
        PIXELFORMAT_LIN_A8R8G8B8 = 0x18280086,
        PIXELFORMAT_X8R8G8B8 = 0x28280186,
        PIXELFORMAT_LIN_X8R8G8B8 = 0x28280086,
        PIXELFORMAT_A8B8G8R8 = 0x1A200186,
        PIXELFORMAT_LIN_A8B8G8R8 = 0x1A200086,
        PIXELFORMAT_X8B8G8R8 = 0x2A200186,
        PIXELFORMAT_LIN_X8B8G8R8 = 0x2A200086,
        PIXELFORMAT_X8L8V8U8 = 0x2A200B86,
        PIXELFORMAT_LIN_X8L8V8U8 = 0x2A200A86,
        PIXELFORMAT_Q8W8V8U8 = 0x1A20AB86,
        PIXELFORMAT_LIN_Q8W8V8U8 = 0x1A20AA86,
        PIXELFORMAT_A2R10G10B10 = 0x182801B6,
        PIXELFORMAT_LIN_A2R10G10B10 = 0x182800B6,
        PIXELFORMAT_X2R10G10B10 = 0x282801B6,
        PIXELFORMAT_LIN_X2R10G10B10 = 0x282800B6,
        PIXELFORMAT_A2B10G10R10 = 0x1A2001B6,
        PIXELFORMAT_LIN_A2B10G10R10 = 0x1A2000B6,
        PIXELFORMAT_A2W10V10U10 = 0x1A202BB6,
        PIXELFORMAT_LIN_A2W10V10U10 = 0x1A202AB6,
        PIXELFORMAT_A16L16 = 0x8000199,
        PIXELFORMAT_LIN_A16L16 = 0x8000099,
        PIXELFORMAT_G16R16 = 0x2D200199,
        PIXELFORMAT_LIN_G16R16 = 0x2D200099,
        PIXELFORMAT_V16U16 = 0x2D20AB99,
        PIXELFORMAT_LIN_V16U16 = 0x2D20AA99,
        PIXELFORMAT_R10G11B11 = 0x282801B7,
        PIXELFORMAT_LIN_R10G11B11 = 0x282800B7,
        PIXELFORMAT_R11G11B10 = 0x282801B8,
        PIXELFORMAT_LIN_R11G11B10 = 0x282800B8,
        PIXELFORMAT_W10V11U11 = 0x2A20ABB7,
        PIXELFORMAT_LIN_W10V11U11 = 0x2A20AAB7,
        PIXELFORMAT_W11V11U10 = 0x2A20ABB8,
        PIXELFORMAT_LIN_W11V11U10 = 0x2A20AAB8,
        PIXELFORMAT_G16R16F = 0x2D22AB9F,
        PIXELFORMAT_LIN_G16R16F = 0x2D22AA9F,
        PIXELFORMAT_G16R16F_EXPAND = 0x2D22AB9C,
        PIXELFORMAT_LIN_G16R16F_EXPAND = 0x2D22AA9C,
        PIXELFORMAT_L32 = 0x280001A1,
        PIXELFORMAT_LIN_L32 = 0x280000A1,
        PIXELFORMAT_R32F = 0x2DA2ABA4,
        PIXELFORMAT_LIN_R32F = 0x2DA2AAA4,
        PIXELFORMAT_A16B16G16R16 = 0x1A20015A,
        PIXELFORMAT_LIN_A16B16G16R16 = 0x1A20005A,
        PIXELFORMAT_Q16W16V16U16 = 0x1A20AB5A,
        PIXELFORMAT_LIN_Q16W16V16U16 = 0x1A20AA5A,
        PIXELFORMAT_A16B16G16R16F = 0x1A22AB60,
        PIXELFORMAT_LIN_A16B16G16R16F = 0x1A22AA60,
        PIXELFORMAT_A16B16G16R16F_EXPAND = 0x1A22AB5D,
        PIXELFORMAT_LIN_A16B16G16R16F_EXPAND = 0x1A22AA5D,
        PIXELFORMAT_A32L32 = 0x80001A2,
        PIXELFORMAT_LIN_A32L32 = 0x80000A2,
        PIXELFORMAT_G32R32 = 0x2D2001A2,
        PIXELFORMAT_LIN_G32R32 = 0x2D2000A2,
        PIXELFORMAT_V32U32 = 0x2D20ABA2,
        PIXELFORMAT_LIN_V32U32 = 0x2D20AAA2,
        PIXELFORMAT_G32R32F = 0x2D22ABA5,
        PIXELFORMAT_LIN_G32R32F = 0x2D22AAA5,
        PIXELFORMAT_A32B32G32R32 = 0x1A2001A3,
        PIXELFORMAT_LIN_A32B32G32R32 = 0x1A2000A3,
        PIXELFORMAT_Q32W32V32U32 = 0x1A20ABA3,
        PIXELFORMAT_LIN_Q32W32V32U32 = 0x1A20AAA3,
        PIXELFORMAT_A32B32G32R32F = 0x1A22ABA6,
        PIXELFORMAT_LIN_A32B32G32R32F = 0x1A22AAA6,
        PIXELFORMAT_A2B10G10R10F_EDRAM = 0x1A2201BF,
        PIXELFORMAT_G16R16_EDRAM = 0x2D20AB8D,
        PIXELFORMAT_A16B16G16R16_EDRAM = 0x1A20AB55,
        PIXELFORMAT_LE_X8R8G8B8 = 0x28280106,
        PIXELFORMAT_LE_A8R8G8B8 = 0x18280106,
        PIXELFORMAT_LE_X2R10G10B10 = 0x28280136,
        PIXELFORMAT_LE_A2R10G10B10 = 0x18280136,
        PIXELFORMAT_INDEX16 = 0x1,
        PIXELFORMAT_INDEX32 = 0x6,
        PIXELFORMAT_VERTEXDATA = 0x8,
        PIXELFORMAT_NA = 0xFFFFFFFF,
        PIXELFORMAT_DXT3A = 0x1A20017A,
        PIXELFORMAT_LIN_DXT3A = 0x1A20007A,
        PIXELFORMAT_DXT3A_1111 = 0x1A20017D,
        PIXELFORMAT_LIN_DXT3A_1111 = 0x1A20007D,
        PIXELFORMAT_DXT5A = 0x1A20017B,
        PIXELFORMAT_LIN_DXT5A = 0x1A20007B,
        PIXELFORMAT_CTX1 = 0x1A20017C,
        PIXELFORMAT_LIN_CTX1 = 0x1A20007C,
        PIXELFORMAT_D24S8 = 0x2D200196,
        PIXELFORMAT_LIN_D24S8 = 0x2D200096,
        PIXELFORMAT_D24X8 = 0x2DA00196,
        PIXELFORMAT_LIN_D24X8 = 0x2DA00096,
        PIXELFORMAT_D24FS8 = 0x1A220197,
        PIXELFORMAT_LIN_D24FS8 = 0x1A220097,
        PIXELFORMAT_D32 = 0x1A2201A1,
        PIXELFORMAT_LIN_D32 = 0x1A2200A1,
        PIXELFORMAT_FORCEENUMSIZEINT = 0x7FFFFFFF,
    };

    inline D3DFORMAT GetD3DFormat(PixelFormat format)
    {
        switch (format)
        {
        // Compressed formats
        case PIXELFORMAT_DXT1:
        case PIXELFORMAT_LIN_DXT1:
            return D3DFMT_DXT1;
        case PIXELFORMAT_DXT3:
        case PIXELFORMAT_LIN_DXT3:
            return D3DFMT_DXT3;
        case PIXELFORMAT_DXT5:
        case PIXELFORMAT_LIN_DXT5:
            return D3DFMT_DXT5;
        case PIXELFORMAT_DXN:
        case PIXELFORMAT_LIN_DXN:
            return D3DFMT_UNKNOWN;

        // 8-bit luminance/alpha formats
        case PIXELFORMAT_A8:
        case PIXELFORMAT_LIN_A8:
            return D3DFMT_A8;
        case PIXELFORMAT_L8:
        case PIXELFORMAT_LIN_L8:
            return D3DFMT_L8;

        // 16-bit color formats
        case PIXELFORMAT_R5G6B5:
        case PIXELFORMAT_LIN_R5G6B5:
            return D3DFMT_R5G6B5;
        case PIXELFORMAT_R6G5B5:
        case PIXELFORMAT_LIN_R6G5B5:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_L6V5U5:
        case PIXELFORMAT_LIN_L6V5U5:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_X1R5G5B5:
        case PIXELFORMAT_LIN_X1R5G5B5:
            return D3DFMT_X1R5G5B5;
        case PIXELFORMAT_A1R5G5B5:
        case PIXELFORMAT_LIN_A1R5G5B5:
            return D3DFMT_A1R5G5B5;
        case PIXELFORMAT_A4R4G4B4:
        case PIXELFORMAT_LIN_A4R4G4B4:
            return D3DFMT_A4R4G4B4;
        case PIXELFORMAT_X4R4G4B4:
        case PIXELFORMAT_LIN_X4R4G4B4:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_Q4W4V4U4:
        case PIXELFORMAT_LIN_Q4W4V4U4:
            return D3DFMT_UNKNOWN;

        // 16-bit formats with luminance & alpha
        case PIXELFORMAT_A8L8:
        case PIXELFORMAT_LIN_A8L8:
            return D3DFMT_A8L8;

        // Some two-component formats
        case PIXELFORMAT_G8R8:
        case PIXELFORMAT_LIN_G8R8:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_V8U8:
        case PIXELFORMAT_LIN_V8U8:
            return D3DFMT_UNKNOWN;

        // Depth formats
        case PIXELFORMAT_D16:
        case PIXELFORMAT_LIN_D16:
            return D3DFMT_D16;

        // 16-bit linear (nonstandard—return unknown)
        case PIXELFORMAT_L16:
        case PIXELFORMAT_LIN_L16:
            return D3DFMT_UNKNOWN;

        // Floating-point formats
        case PIXELFORMAT_R16F:
        case PIXELFORMAT_LIN_R16F:
            return D3DFMT_R16F;
        case PIXELFORMAT_R16F_EXPAND:
        case PIXELFORMAT_LIN_R16F_EXPAND:
            return D3DFMT_UNKNOWN;

        // YUV variants and packed formats
        case PIXELFORMAT_UYVY:
        case PIXELFORMAT_LIN_UYVY:
        case PIXELFORMAT_LE_UYVY:
        case PIXELFORMAT_LE_LIN_UYVY:
            return D3DFMT_UYVY;
        case PIXELFORMAT_G8R8_G8B8:
        case PIXELFORMAT_LIN_G8R8_G8B8:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_R8G8_B8G8:
        case PIXELFORMAT_LIN_R8G8_B8G8:
            return D3DFMT_R8G8_B8G8;
        case PIXELFORMAT_YUY2:
        case PIXELFORMAT_LIN_YUY2:
        case PIXELFORMAT_LE_YUY2:
        case PIXELFORMAT_LE_LIN_YUY2:
            return D3DFMT_YUY2;

        // 32-bit color formats
        case PIXELFORMAT_A8R8G8B8:
        case PIXELFORMAT_LIN_A8R8G8B8:
            return D3DFMT_A8R8G8B8;
        case PIXELFORMAT_X8R8G8B8:
        case PIXELFORMAT_LIN_X8R8G8B8:
            return D3DFMT_X8R8G8B8;
        case PIXELFORMAT_A8B8G8R8:
        case PIXELFORMAT_LIN_A8B8G8R8:
            return D3DFMT_A8B8G8R8;
        case PIXELFORMAT_X8B8G8R8:
        case PIXELFORMAT_LIN_X8B8G8R8:
            return D3DFMT_X8B8G8R8;

        // Other 32-bit formats (nonstandard)
        case PIXELFORMAT_X8L8V8U8:
        case PIXELFORMAT_LIN_X8L8V8U8:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_Q8W8V8U8:
        case PIXELFORMAT_LIN_Q8W8V8U8:
            return D3DFMT_UNKNOWN;

        // 10-/11-/10-bit and related formats
        case PIXELFORMAT_A2R10G10B10:
        case PIXELFORMAT_LIN_A2R10G10B10:
            return D3DFMT_A2R10G10B10;
        case PIXELFORMAT_X2R10G10B10:
        case PIXELFORMAT_LIN_X2R10G10B10:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_A2B10G10R10:
        case PIXELFORMAT_LIN_A2B10G10R10:
            return D3DFMT_A2B10G10R10;
        case PIXELFORMAT_A2W10V10U10:
        case PIXELFORMAT_LIN_A2W10V10U10:
            return D3DFMT_UNKNOWN;

        // 32-bit luminance / alpha formats
        case PIXELFORMAT_A16L16:
        case PIXELFORMAT_LIN_A16L16:
            return D3DFMT_UNKNOWN;

        // 32-bit color formats with 16 bits per component
        case PIXELFORMAT_G16R16:
        case PIXELFORMAT_LIN_G16R16:
            return D3DFMT_G16R16;
        case PIXELFORMAT_V16U16:
        case PIXELFORMAT_LIN_V16U16:
            return D3DFMT_UNKNOWN;

        // 10/11/10 packed formats
        case PIXELFORMAT_R10G11B11:
        case PIXELFORMAT_LIN_R10G11B11:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_R11G11B10:
        case PIXELFORMAT_LIN_R11G11B10:
            return D3DFMT_UNKNOWN;

        // More exotic formats
        case PIXELFORMAT_W10V11U11:
        case PIXELFORMAT_LIN_W10V11U11:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_W11V11U10:
        case PIXELFORMAT_LIN_W11V11U10:
            return D3DFMT_UNKNOWN;

        // Floating-point high-precision formats
        case PIXELFORMAT_G16R16F:
        case PIXELFORMAT_LIN_G16R16F:
            return D3DFMT_G16R16F;
        case PIXELFORMAT_G16R16F_EXPAND:
        case PIXELFORMAT_LIN_G16R16F_EXPAND:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_L32:
        case PIXELFORMAT_LIN_L32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_R32F:
        case PIXELFORMAT_LIN_R32F:
            return D3DFMT_R32F;
        case PIXELFORMAT_A16B16G16R16:
        case PIXELFORMAT_LIN_A16B16G16R16:
            return D3DFMT_A16B16G16R16;
        case PIXELFORMAT_Q16W16V16U16:
        case PIXELFORMAT_LIN_Q16W16V16U16:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_A16B16G16R16F:
        case PIXELFORMAT_LIN_A16B16G16R16F:
            return D3DFMT_A16B16G16R16F;
        case PIXELFORMAT_A16B16G16R16F_EXPAND:
        case PIXELFORMAT_LIN_A16B16G16R16F_EXPAND:
            return D3DFMT_UNKNOWN;

        // 64-bit formats
        case PIXELFORMAT_A32L32:
        case PIXELFORMAT_LIN_A32L32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_G32R32:
        case PIXELFORMAT_LIN_G32R32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_V32U32:
        case PIXELFORMAT_LIN_V32U32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_G32R32F:
        case PIXELFORMAT_LIN_G32R32F:
            return D3DFMT_G32R32F;
        case PIXELFORMAT_A32B32G32R32:
        case PIXELFORMAT_LIN_A32B32G32R32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_Q32W32V32U32:
        case PIXELFORMAT_LIN_Q32W32V32U32:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_A32B32G32R32F:
        case PIXELFORMAT_LIN_A32B32G32R32F:
            return D3DFMT_UNKNOWN;

        // EDRAM / platform‐specific formats
        case PIXELFORMAT_A2B10G10R10F_EDRAM:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_G16R16_EDRAM:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_A16B16G16R16_EDRAM:
            return D3DFMT_UNKNOWN;

        // Little-endian specific cases
        case PIXELFORMAT_LE_X8R8G8B8:
            return D3DFMT_X8R8G8B8;
        case PIXELFORMAT_LE_A8R8G8B8:
            return D3DFMT_A8R8G8B8;
        case PIXELFORMAT_LE_X2R10G10B10:
            return D3DFMT_UNKNOWN;
        case PIXELFORMAT_LE_A2R10G10B10:
            return D3DFMT_A2R10G10B10;

        // Special or unsupported types
        case PIXELFORMAT_INDEX16:
        case PIXELFORMAT_INDEX32:
        case PIXELFORMAT_VERTEXDATA:
        case PIXELFORMAT_NA:
            return D3DFMT_UNKNOWN;

        // Special variants of DXT formats
        case PIXELFORMAT_DXT3A:
        case PIXELFORMAT_LIN_DXT3A:
            return D3DFMT_DXT3;
        case PIXELFORMAT_DXT3A_1111:
        case PIXELFORMAT_LIN_DXT3A_1111:
            return D3DFMT_DXT3;
        case PIXELFORMAT_DXT5A:
        case PIXELFORMAT_LIN_DXT5A:
            return D3DFMT_DXT5;
        case PIXELFORMAT_CTX1:
        case PIXELFORMAT_LIN_CTX1:
            return D3DFMT_UNKNOWN;

        // Depth/stencil surface formats
        case PIXELFORMAT_D24S8:
        case PIXELFORMAT_LIN_D24S8:
            return D3DFMT_D24S8;
        case PIXELFORMAT_D24X8:
        case PIXELFORMAT_LIN_D24X8:
            return D3DFMT_D24X8;
        case PIXELFORMAT_D24FS8:
        case PIXELFORMAT_LIN_D24FS8:
            return D3DFMT_D24FS8;
        case PIXELFORMAT_D32:
        case PIXELFORMAT_LIN_D32:
            return D3DFMT_D32;

        default:
            return D3DFMT_UNKNOWN;
        }
    }

    struct TextureParameters
    {
        be<renderengine::Type> type;
        be<unsigned int> flags;
        be<unsigned int> width;
        be<unsigned int> height;
        be<unsigned int> depth;
        be<unsigned int> mipLevels;
        be<PixelFormat> format;
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

    inline std::map<uint32_t, GuestTexture *> g_textureMap;
    inline ThreadSafeQueue<Batches::BatchInfo *> batchQueue;
}