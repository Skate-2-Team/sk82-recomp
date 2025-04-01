#pragma once

#include <windows.h>
#include <queue>
#include "ppc/ppc_recomp_shared.h"
#include "video.h"
#include "kernel/function.h"
#include "kernel/heap.h"
#include "kernel/xbox.h"
#include "xex.h"
#include <DirectXMath.h>

namespace VideoHooks
{
    inline std::chrono::steady_clock::time_point m_frameStart;

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

    enum XBOXPRIMITIVETYPE : __int32
    {
        XD3DPT_POINTLIST = 0x1,
        XD3DPT_LINELIST = 0x2,
        XD3DPT_LINESTRIP = 0x3,
        XD3DPT_TRIANGLELIST = 0x4,
        XD3DPT_TRIANGLEFAN = 0x5,
        XD3DPT_TRIANGLESTRIP = 0x6,
        XD3DPT_RECTLIST = 0x8,
        XD3DPT_QUADLIST = 0xD,
        XD3DPT_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum _D3DBLOCKTYPE : __int32
    {
        D3DBLOCKTYPE_NONE = 0x0,
        D3DBLOCKTYPE_PRIMARY_OVERRUN = 0x1,
        D3DBLOCKTYPE_SECONDARY_OVERRUN = 0x2,
        D3DBLOCKTYPE_SWAP_THROTTLE = 0x3,
        D3DBLOCKTYPE_BLOCK_UNTIL_IDLE = 0x4,
        D3DBLOCKTYPE_BLOCK_UNTIL_NOT_BUSY = 0x5,
        D3DBLOCKTYPE_BLOCK_ON_FENCE = 0x6,
        D3DBLOCKTYPE_VERTEX_SHADER_RELEASE = 0x7,
        D3DBLOCKTYPE_PIXEL_SHADER_RELEASE = 0x8,
        D3DBLOCKTYPE_VERTEX_BUFFER_RELEASE = 0x9,
        D3DBLOCKTYPE_VERTEX_BUFFER_LOCK = 0xA,
        D3DBLOCKTYPE_INDEX_BUFFER_RELEASE = 0xB,
        D3DBLOCKTYPE_INDEX_BUFFER_LOCK = 0xC,
        D3DBLOCKTYPE_TEXTURE_RELEASE = 0xD,
        D3DBLOCKTYPE_TEXTURE_LOCK = 0xE,
        D3DBLOCKTYPE_COMMAND_BUFFER_RELEASE = 0xF,
        D3DBLOCKTYPE_COMMAND_BUFFER_LOCK = 0x10,
        D3DBLOCKTYPE_CONSTANT_BUFFER_RELEASE = 0x11,
        D3DBLOCKTYPE_CONSTANT_BUFFER_LOCK = 0x12,
        D3DBLOCKTYPE_MAX = 0x13,
    };

    HRESULT Direct3D_CreateDevice(
        unsigned int Adapter,
        int DeviceType,
        void *pUnused,
        unsigned int BehaviorFlags,
        void *pPresentationParameters,
        void **ppReturnedDeviceInterface);

    void *D3DDevice_CreateSurface(
        unsigned int Width,
        unsigned int Height,
        _D3DFORMAT D3DFormat,
        _D3DMULTISAMPLE_TYPE MultiSample,
        const _D3DSURFACE_PARAMETERS *pParameters);

    void *D3DDevice_CreateTexture(
        unsigned int Width,
        unsigned int Height,
        unsigned int Depth,
        unsigned int Levels,
        unsigned int Usage,
        signed int D3DFormat,
        unsigned int Pool,
        _D3DRESOURCETYPE D3DType);

    void D3DDevice_Swap(
        void *pDevice,
        void *pFrontBuffer,
        const void *pParameters);
}