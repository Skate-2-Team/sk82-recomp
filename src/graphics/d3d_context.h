#pragma once

#include <d3d9.h>
#include <string>
#include <map>

#include "xex.h"
#include "log.h"

#define XD3DCLEAR_TARGET0 0x00000001l                                                                        /* Clear target surface 0 */
#define XD3DCLEAR_TARGET1 0x00000002l                                                                        /* Clear target surface 1 */
#define XD3DCLEAR_TARGET2 0x00000004l                                                                        /* Clear target surface 2 */
#define XD3DCLEAR_TARGET3 0x00000008l                                                                        /* Clear target surface 3 */
#define XD3DCLEAR_ALLTARGETS (XD3DCLEAR_TARGET0 | XD3DCLEAR_TARGET1 | XD3DCLEAR_TARGET2 | XD3DCLEAR_TARGET3) /* Clear all target surfaces */
#define XD3DCLEAR_TARGET XD3DCLEAR_ALLTARGETS

#define XD3DCLEAR_ZBUFFER 0x00000010l        /* Clear target z buffer */
#define XD3DCLEAR_STENCIL 0x00000020l        /* Clear stencil planes */
#define XD3DCLEAR_HISTENCIL_CULL 0x00000040l /* Clear hi-stencil to cull all pixels */
#define XD3DCLEAR_HISTENCIL_PASS 0x00000080l /* Clear hi-stencil to pass all pixels */

namespace GuestD3D
{
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

    enum BlockType : uint32_t
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

    union BlendState
    {
        struct
        {
            unsigned __int32 SrcBlend : 5;
            unsigned __int32 BlendOp : 3;
            unsigned __int32 DestBlend : 8;
            unsigned __int32 SrcBlendAlpha : 5;
            unsigned __int32 BlendOpAlpha : 3;
            unsigned __int32 DestBlendAlpha : 8;
        };

        uint32_t dword;
    };

    enum CullMode : __int32
    {
        D3DCULL_NONE = 0x0,
        D3DCULL_CW = 0x2,
        D3DCULL_CCW = 0x6,
        D3DCULL_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum FillMode : uint32_t
    {
        D3DFILL_POINT = 0x1,
        D3DFILL_WIREFRAME = 0x25,
        D3DFILL_SOLID = 0x0,
        D3DFILL_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum Blend : uint32_t
    {
        D3DBLEND_ZERO = 0x0,
        D3DBLEND_ONE = 0x1,
        D3DBLEND_SRCCOLOR = 0x4,
        D3DBLEND_INVSRCCOLOR = 0x5,
        D3DBLEND_SRCALPHA = 0x6,
        D3DBLEND_INVSRCALPHA = 0x7,
        D3DBLEND_DESTCOLOR = 0x8,
        D3DBLEND_INVDESTCOLOR = 0x9,
        D3DBLEND_DESTALPHA = 0xA,
        D3DBLEND_INVDESTALPHA = 0xB,
        D3DBLEND_BLENDFACTOR = 0xC,
        D3DBLEND_INVBLENDFACTOR = 0xD,
        D3DBLEND_CONSTANTALPHA = 0xE,
        D3DBLEND_INVCONSTANTALPHA = 0xF,
        D3DBLEND_SRCALPHASAT = 0x10,
        D3DBLEND_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum BlendOp : uint32_t
    {
        D3DBLENDOP_ADD = 0x0,
        D3DBLENDOP_SUBTRACT = 0x1,
        D3DBLENDOP_MIN = 0x2,
        D3DBLENDOP_MAX = 0x3,
        D3DBLENDOP_REVSUBTRACT = 0x4,
        D3DBLENDOP_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum StencilOp : uint32_t
    {
        D3DSTENCILOP_KEEP = 0x0,
        D3DSTENCILOP_ZERO = 0x1,
        D3DSTENCILOP_REPLACE = 0x2,
        D3DSTENCILOP_INCRSAT = 0x3,
        D3DSTENCILOP_DECRSAT = 0x4,
        D3DSTENCILOP_INVERT = 0x5,
        D3DSTENCILOP_INCR = 0x6,
        D3DSTENCILOP_DECR = 0x7,
        D3DSTENCILOP_FORCE_DWORD = 0x7FFFFFFF,
    };

    enum CmpFunc : uint32_t
    {
        D3DCMP_NEVER = 0x0,
        D3DCMP_LESS = 0x1,
        D3DCMP_EQUAL = 0x2,
        D3DCMP_LESSEQUAL = 0x3,
        D3DCMP_GREATER = 0x4,
        D3DCMP_NOTEQUAL = 0x5,
        D3DCMP_GREATEREQUAL = 0x6,
        D3DCMP_ALWAYS = 0x7,
        D3DCMP_FORCE_DWORD = 0x7FFFFFFF,
    };

    typedef D3DDECLUSAGE DeclUsage;
    typedef D3DDECLMETHOD DeclMethod;

    enum DeclType : uint32_t
    {
        D3DDECLTYPE_FLOAT1 = 0x2C83A4,
        D3DDECLTYPE_FLOAT2 = 0x2C23A5,
        D3DDECLTYPE_FLOAT3 = 0x2A23B9,
        D3DDECLTYPE_FLOAT4 = 0x1A23A6,
        D3DDECLTYPE_INT1 = 0x2C83A1,
        D3DDECLTYPE_INT2 = 0x2C23A2,
        D3DDECLTYPE_INT4 = 0x1A23A3,
        D3DDECLTYPE_UINT1 = 0x2C82A1,
        D3DDECLTYPE_UINT2 = 0x2C22A2,
        D3DDECLTYPE_UINT4 = 0x1A22A3,
        D3DDECLTYPE_INT1N = 0x2C81A1,
        D3DDECLTYPE_INT2N = 0x2C21A2,
        D3DDECLTYPE_INT4N = 0x1A21A3,
        D3DDECLTYPE_UINT1N = 0x2C80A1,
        D3DDECLTYPE_UINT2N = 0x2C20A2,
        D3DDECLTYPE_UINT4N = 0x1A20A3,
        D3DDECLTYPE_D3DCOLOR = 0x182886,
        D3DDECLTYPE_UBYTE4 = 0x1A2286,
        D3DDECLTYPE_BYTE4 = 0x1A2386,
        D3DDECLTYPE_UBYTE4N = 0x1A2086,
        D3DDECLTYPE_BYTE4N = 0x1A2186,
        D3DDECLTYPE_SHORT2 = 0x2C2359,
        D3DDECLTYPE_SHORT4 = 0x1A235A,
        D3DDECLTYPE_USHORT2 = 0x2C2259,
        D3DDECLTYPE_USHORT4 = 0x1A225A,
        D3DDECLTYPE_SHORT2N = 0x2C2159,
        D3DDECLTYPE_SHORT4N = 0x1A215A,
        D3DDECLTYPE_USHORT2N = 0x2C2059,
        D3DDECLTYPE_USHORT4N = 0x1A205A,
        D3DDECLTYPE_UDEC3 = 0x2A2287,
        D3DDECLTYPE_DEC3 = 0x2A2387,
        D3DDECLTYPE_UDEC3N = 0x2A2087,
        D3DDECLTYPE_DEC3N = 0x2A2187,
        D3DDECLTYPE_UDEC4 = 0x1A2287,
        D3DDECLTYPE_DEC4 = 0x1A2387,
        D3DDECLTYPE_UDEC4N = 0x1A2087,
        D3DDECLTYPE_DEC4N = 0x1A2187,
        D3DDECLTYPE_UHEND3 = 0x2A2290,
        D3DDECLTYPE_HEND3 = 0x2A2390,
        D3DDECLTYPE_UHEND3N = 0x2A2090,
        D3DDECLTYPE_HEND3N = 0x2A2190,
        D3DDECLTYPE_UDHEN3 = 0x2A2291,
        D3DDECLTYPE_DHEN3 = 0x2A2391,
        D3DDECLTYPE_UDHEN3N = 0x2A2091,
        D3DDECLTYPE_DHEN3N = 0x2A2191,
        D3DDECLTYPE_FLOAT16_2 = 0x2C235F,
        D3DDECLTYPE_FLOAT16_4 = 0x1A2360,
        D3DDECLTYPE_UNUSED = 0xFFFFFFFF,
    };

    inline std::string GetDeclUsageName(uint32_t type)
    {
        switch (type)
        {
        case D3DDECLUSAGE_POSITION:
            return "POSITION";
        case D3DDECLUSAGE_BLENDWEIGHT:
            return "BLENDWEIGHT";
        case D3DDECLUSAGE_BLENDINDICES:
            return "BLENDINDICES";
        case D3DDECLUSAGE_NORMAL:
            return "NORMAL";
        case D3DDECLUSAGE_PSIZE:
            return "PSIZE";
        case D3DDECLUSAGE_TEXCOORD:
            return "TEXCOORD";
        case D3DDECLUSAGE_TANGENT:
            return "TANGENT";
        case D3DDECLUSAGE_BINORMAL:
            return "BINORMAL";
        case D3DDECLUSAGE_TESSFACTOR:
            return "TESSFACTOR";
        case D3DDECLUSAGE_POSITIONT:
            return "POSITIONT";
        case D3DDECLUSAGE_COLOR:
            return "COLOR";
        case D3DDECLUSAGE_FOG:
            return "FOG";
        case D3DDECLUSAGE_DEPTH:
            return "DEPTH";
        case D3DDECLUSAGE_SAMPLE:
            return "SAMPLE";
        default:
            return "UNKNOWN";
        }
    }

    inline std::string GetDeclMethodName(uint32_t type)
    {
        switch (type)
        {
        case D3DDECLMETHOD_DEFAULT:
            return "DEFAULT";
        case D3DDECLMETHOD_PARTIALU:
            return "PARTIALU";
        case D3DDECLMETHOD_PARTIALV:
            return "PARTIALV";
        case D3DDECLMETHOD_CROSSUV:
            return "CROSSUV";
        case D3DDECLMETHOD_UV:
            return "UV";
        case D3DDECLMETHOD_LOOKUP:
            return "LOOKUP";
        case D3DDECLMETHOD_LOOKUPPRESAMPLED:
            return "LOOKUPPRESAMPLED";
        default:
            return "UNKNOWN";
        }
    }

    inline std::string GetDeclTypeName(uint32_t type)
    {
        switch (type)
        {
        case D3DDECLTYPE_FLOAT1:
            return "FLOAT1";
        case D3DDECLTYPE_FLOAT2:
            return "FLOAT2";
        case D3DDECLTYPE_FLOAT3:
            return "FLOAT3";
        case D3DDECLTYPE_FLOAT4:
            return "FLOAT4";
        case D3DDECLTYPE_INT1:
            return "INT1";
        case D3DDECLTYPE_INT2:
            return "INT2";
        case D3DDECLTYPE_INT4:
            return "INT4";
        case D3DDECLTYPE_UINT1:
            return "UINT1";
        case D3DDECLTYPE_UINT2:
            return "UINT2";
        case D3DDECLTYPE_UINT4:
            return "UINT4";
        case D3DDECLTYPE_INT1N:
            return "INT1N";
        case D3DDECLTYPE_INT2N:
            return "INT2N";
        case D3DDECLTYPE_INT4N:
            return "INT4N";
        case D3DDECLTYPE_UINT1N:
            return "UINT1N";
        case D3DDECLTYPE_UINT2N:
            return "UINT2N";
        case D3DDECLTYPE_UINT4N:
            return "UINT4N";
        case D3DDECLTYPE_D3DCOLOR:
            return "D3DCOLOR";
        case D3DDECLTYPE_UBYTE4:
            return "UBYTE4";
        case D3DDECLTYPE_BYTE4:
            return "BYTE4";
        case D3DDECLTYPE_UBYTE4N:
            return "UBYTE4N";
        case D3DDECLTYPE_BYTE4N:
            return "BYTE4N";
        case D3DDECLTYPE_SHORT2:
            return "SHORT2";
        case D3DDECLTYPE_SHORT4:
            return "SHORT4";
        case D3DDECLTYPE_USHORT2:
            return "USHORT2";
        case D3DDECLTYPE_USHORT4:
            return "USHORT4";
        case D3DDECLTYPE_SHORT2N:
            return "SHORT2N";
        case D3DDECLTYPE_SHORT4N:
            return "SHORT4N";
        case D3DDECLTYPE_USHORT2N:
            return "USHORT2N";
        case D3DDECLTYPE_USHORT4N:
            return "USHORT4N";
        case D3DDECLTYPE_UDEC3:
            return "UDEC3";
        case D3DDECLTYPE_DEC3:
            return "DEC3";
        case D3DDECLTYPE_UDEC3N:
            return "UDEC3N";
        case D3DDECLTYPE_DEC3N:
            return "DEC3N";
        case D3DDECLTYPE_UDEC4:
            return "UDEC4";
        case D3DDECLTYPE_DEC4:
            return "DEC4";
        case D3DDECLTYPE_UDEC4N:
            return "UDEC4N";
        case D3DDECLTYPE_DEC4N:
            return "DEC4N";
        case D3DDECLTYPE_UHEND3:
            return "UHEND3";
        case D3DDECLTYPE_HEND3:
            return "HEND3";
        case D3DDECLTYPE_UHEND3N:
            return "UHEND3N";
        case D3DDECLTYPE_HEND3N:
            return "HEND3N";
        case D3DDECLTYPE_UDHEN3:
            return "UDHEN3";
        case D3DDECLTYPE_DHEN3:
            return "DHEN3";
        case D3DDECLTYPE_UDHEN3N:
            return "UDHEN3N";
        case D3DDECLTYPE_DHEN3N:
            return "DHEN3N";
        case D3DDECLTYPE_FLOAT16_2:
            return "FLOAT16_2";
        case D3DDECLTYPE_FLOAT16_4:
            return "FLOAT16_4";
        case D3DDECLTYPE_UNUSED:
            return "UNUSED";
        default:
            return "UNKNOWN";
        }
    }

    enum PrimitiveType : int
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

    struct GPUTEXTURESIZE_1D
    {
        uint32_t Width : 24;
    };

    struct GPUTEXTURESIZE_2D
    {
        uint32_t Width : 13;
        uint32_t Height : 13;
    };

    struct GPUTEXTURESIZE_3D
    {
        uint32_t Width : 11;
        uint32_t Height : 11;
        uint32_t Depth : 10;
    };

    struct GPUTEXTURESIZE_STACK
    {
        uint32_t Width : 13;
        uint32_t Height : 13;
        uint32_t Depth : 6;
    };

    union GPUTEXTURE_FETCH_CONSTANT
    {
        struct
        {
            // DWORD 1
            uint32_t Type : 2;
            uint32_t SignX : 2;
            uint32_t SignY : 2;
            uint32_t SignZ : 2;
            uint32_t SignW : 2;
            uint32_t ClampX : 3;
            uint32_t ClampY : 3;
            uint32_t ClampZ : 3;
            uint32_t : 3;
            uint32_t Pitch : 9;
            uint32_t Tiled : 1;

            // DWORD 2
            uint32_t DataFormat : 6;
            uint32_t Endian : 2;
            uint32_t RequestSize : 2;
            uint32_t Stacked : 1;
            uint32_t ClampPolicy : 1;
            uint32_t BaseAddress : 20;

            union
            {
                GPUTEXTURESIZE_1D OneD;
                GPUTEXTURESIZE_2D TwoD;
                GPUTEXTURESIZE_3D ThreeD;
                GPUTEXTURESIZE_STACK Stack;
            } Size;

            uint32_t NumFormat : 1;
            uint32_t SwizzleX : 3;
            uint32_t SwizzleY : 3;
            uint32_t SwizzleZ : 3;
            uint32_t SwizzleW : 3;
            int ExpAdjust : 6;
            uint32_t MagFilter : 2;
            uint32_t MinFilter : 2;
            uint32_t MipFilter : 2;
            uint32_t AnisoFilter : 3;
            uint32_t : 3;
            uint32_t BorderSize : 1;
            uint32_t VolMagFilter : 1;
            uint32_t VolMinFilter : 1;
            uint32_t MinMipLevel : 4;
            uint32_t MaxMipLevel : 4;
            uint32_t MagAnisoWalk : 1;
            uint32_t MinAnisoWalk : 1;
            int LODBias : 10;
            int GradExpAdjustH : 5;
            int GradExpAdjustV : 5;
            uint32_t BorderColor : 2;
            uint32_t ForceBCWToMax : 1;
            uint32_t TriClamp : 2;
            int AnisoBias : 4;
            uint32_t Dimension : 2;
            uint32_t PackedMips : 1;
            uint32_t MipAddress : 20;
        };

        uint32_t dword[6];
    };

    union GPUVERTEX_FETCH_CONSTANT
    {
        struct
        {
            uint32_t Type : 2;
            uint32_t BaseAddress : 30;
            uint32_t Endian : 2;
            uint32_t Size : 24;
            uint32_t AddressClamp : 1;
            uint32_t : 1;
            uint32_t RequestSize : 2;
            uint32_t ClampDisable : 2;
        };

        uint32_t dword[2];
    };

    union GPUFETCH_CONSTANT
    {
        GPUTEXTURE_FETCH_CONSTANT Texture;
        GPUVERTEX_FETCH_CONSTANT Vertex[3];
    };

    struct Vector4
    {
        union
        {
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };

            float v[4];
            unsigned int u[4];
        };
    };

    struct TagCollection
    {
        uint64_t m_Mask[5];
    };

    struct Constants
    {
        union
        {
            GPUFETCH_CONSTANT Fetch[32];

            struct
            {
                GPUTEXTURE_FETCH_CONSTANT TextureFetch[26];
                GPUVERTEX_FETCH_CONSTANT VertexFetch[18];
            };
        };

        union
        {
            Vector4 Alu[512];

            struct
            {
                Vector4 VertexShaderF[256];
                Vector4 PixelShaderF[256];
            };
        };

        union
        {
            unsigned int Flow[40];

            struct
            {
                unsigned int VertexShaderB[4];
                unsigned int PixelShaderB[4];
                unsigned int VertexShaderI[16];
                unsigned int PixelShaderI[16];
            };
        };
    };

    struct VertexElement
    {
        be<uint16_t> Stream;
        be<uint16_t> Offset;
        be<uint32_t> Type;
        be<uint8_t> Method;
        be<uint8_t> Usage;
        be<uint8_t> UsageIndex;
        be<uint8_t> Reserved;
    };
}

inline D3DFORMAT GetD3DFormat(GuestD3D::PixelFormat p_format)
{
    switch (p_format)
    {
    // Compressed formats
    case GuestD3D::PIXELFORMAT_DXT1:
    case GuestD3D::PIXELFORMAT_LIN_DXT1:
        return D3DFMT_DXT1;
    case GuestD3D::PIXELFORMAT_DXT3:
    case GuestD3D::PIXELFORMAT_LIN_DXT3:
        return D3DFMT_DXT3;
    case GuestD3D::PIXELFORMAT_DXT5:
    case GuestD3D::PIXELFORMAT_LIN_DXT5:
        return D3DFMT_DXT5;
    case GuestD3D::PIXELFORMAT_DXN:
    case GuestD3D::PIXELFORMAT_LIN_DXN:
        return D3DFMT_UNKNOWN;

    // 8-bit luminance/alpha formats
    case GuestD3D::PIXELFORMAT_A8:
    case GuestD3D::PIXELFORMAT_LIN_A8:
        return D3DFMT_A8;
    case GuestD3D::PIXELFORMAT_L8:
    case GuestD3D::PIXELFORMAT_LIN_L8:
        return D3DFMT_L8;

    // 16-bit color formats
    case GuestD3D::PIXELFORMAT_R5G6B5:
    case GuestD3D::PIXELFORMAT_LIN_R5G6B5:
        return D3DFMT_R5G6B5;
    case GuestD3D::PIXELFORMAT_R6G5B5:
    case GuestD3D::PIXELFORMAT_LIN_R6G5B5:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_L6V5U5:
    case GuestD3D::PIXELFORMAT_LIN_L6V5U5:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_X1R5G5B5:
    case GuestD3D::PIXELFORMAT_LIN_X1R5G5B5:
        return D3DFMT_X1R5G5B5;
    case GuestD3D::PIXELFORMAT_A1R5G5B5:
    case GuestD3D::PIXELFORMAT_LIN_A1R5G5B5:
        return D3DFMT_A1R5G5B5;
    case GuestD3D::PIXELFORMAT_A4R4G4B4:
    case GuestD3D::PIXELFORMAT_LIN_A4R4G4B4:
        return D3DFMT_A4R4G4B4;
    case GuestD3D::PIXELFORMAT_X4R4G4B4:
    case GuestD3D::PIXELFORMAT_LIN_X4R4G4B4:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_Q4W4V4U4:
    case GuestD3D::PIXELFORMAT_LIN_Q4W4V4U4:
        return D3DFMT_UNKNOWN;

    // 16-bit formats with luminance & alpha
    case GuestD3D::PIXELFORMAT_A8L8:
    case GuestD3D::PIXELFORMAT_LIN_A8L8:
        return D3DFMT_A8L8;

    // Some two-component formats
    case GuestD3D::PIXELFORMAT_G8R8:
    case GuestD3D::PIXELFORMAT_LIN_G8R8:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_V8U8:
    case GuestD3D::PIXELFORMAT_LIN_V8U8:
        return D3DFMT_UNKNOWN;

    // Depth formats
    case GuestD3D::PIXELFORMAT_D16:
    case GuestD3D::PIXELFORMAT_LIN_D16:
        return D3DFMT_D16;

    // 16-bit linear (nonstandard—return unknown)
    case GuestD3D::PIXELFORMAT_L16:
    case GuestD3D::PIXELFORMAT_LIN_L16:
        return D3DFMT_UNKNOWN;

    // Floating-point formats
    case GuestD3D::PIXELFORMAT_R16F:
    case GuestD3D::PIXELFORMAT_LIN_R16F:
        return D3DFMT_R16F;
    case GuestD3D::PIXELFORMAT_R16F_EXPAND:
    case GuestD3D::PIXELFORMAT_LIN_R16F_EXPAND:
        return D3DFMT_UNKNOWN;

    // YUV variants and packed formats
    case GuestD3D::PIXELFORMAT_UYVY:
    case GuestD3D::PIXELFORMAT_LIN_UYVY:
    case GuestD3D::PIXELFORMAT_LE_UYVY:
    case GuestD3D::PIXELFORMAT_LE_LIN_UYVY:
        return D3DFMT_UYVY;
    case GuestD3D::PIXELFORMAT_G8R8_G8B8:
    case GuestD3D::PIXELFORMAT_LIN_G8R8_G8B8:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_R8G8_B8G8:
    case GuestD3D::PIXELFORMAT_LIN_R8G8_B8G8:
        return D3DFMT_R8G8_B8G8;
    case GuestD3D::PIXELFORMAT_YUY2:
    case GuestD3D::PIXELFORMAT_LIN_YUY2:
    case GuestD3D::PIXELFORMAT_LE_YUY2:
    case GuestD3D::PIXELFORMAT_LE_LIN_YUY2:
        return D3DFMT_YUY2;

    // 32-bit color formats
    case GuestD3D::PIXELFORMAT_A8R8G8B8:
    case GuestD3D::PIXELFORMAT_LIN_A8R8G8B8:
        return D3DFMT_A8R8G8B8;
    case GuestD3D::PIXELFORMAT_X8R8G8B8:
    case GuestD3D::PIXELFORMAT_LIN_X8R8G8B8:
        return D3DFMT_X8R8G8B8;
    case GuestD3D::PIXELFORMAT_A8B8G8R8:
    case GuestD3D::PIXELFORMAT_LIN_A8B8G8R8:
        return D3DFMT_A8B8G8R8;
    case GuestD3D::PIXELFORMAT_X8B8G8R8:
    case GuestD3D::PIXELFORMAT_LIN_X8B8G8R8:
        return D3DFMT_X8B8G8R8;

    // Other 32-bit formats (nonstandard)
    case GuestD3D::PIXELFORMAT_X8L8V8U8:
    case GuestD3D::PIXELFORMAT_LIN_X8L8V8U8:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_Q8W8V8U8:
    case GuestD3D::PIXELFORMAT_LIN_Q8W8V8U8:
        return D3DFMT_UNKNOWN;

    // 10-/11-/10-bit and related formats
    case GuestD3D::PIXELFORMAT_A2R10G10B10:
    case GuestD3D::PIXELFORMAT_LIN_A2R10G10B10:
        return D3DFMT_A2R10G10B10;
    case GuestD3D::PIXELFORMAT_X2R10G10B10:
    case GuestD3D::PIXELFORMAT_LIN_X2R10G10B10:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_A2B10G10R10:
    case GuestD3D::PIXELFORMAT_LIN_A2B10G10R10:
        return D3DFMT_A2B10G10R10;
    case GuestD3D::PIXELFORMAT_A2W10V10U10:
    case GuestD3D::PIXELFORMAT_LIN_A2W10V10U10:
        return D3DFMT_UNKNOWN;

    // 32-bit luminance / alpha formats
    case GuestD3D::PIXELFORMAT_A16L16:
    case GuestD3D::PIXELFORMAT_LIN_A16L16:
        return D3DFMT_UNKNOWN;

    // 32-bit color formats with 16 bits per component
    case GuestD3D::PIXELFORMAT_G16R16:
    case GuestD3D::PIXELFORMAT_LIN_G16R16:
        return D3DFMT_G16R16;
    case GuestD3D::PIXELFORMAT_V16U16:
    case GuestD3D::PIXELFORMAT_LIN_V16U16:
        return D3DFMT_UNKNOWN;

    // 10/11/10 packed formats
    case GuestD3D::PIXELFORMAT_R10G11B11:
    case GuestD3D::PIXELFORMAT_LIN_R10G11B11:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_R11G11B10:
    case GuestD3D::PIXELFORMAT_LIN_R11G11B10:
        return D3DFMT_UNKNOWN;

    // More exotic formats
    case GuestD3D::PIXELFORMAT_W10V11U11:
    case GuestD3D::PIXELFORMAT_LIN_W10V11U11:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_W11V11U10:
    case GuestD3D::PIXELFORMAT_LIN_W11V11U10:
        return D3DFMT_UNKNOWN;

    // Floating-point high-precision formats
    case GuestD3D::PIXELFORMAT_G16R16F:
    case GuestD3D::PIXELFORMAT_LIN_G16R16F:
        return D3DFMT_G16R16F;
    case GuestD3D::PIXELFORMAT_G16R16F_EXPAND:
    case GuestD3D::PIXELFORMAT_LIN_G16R16F_EXPAND:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_L32:
    case GuestD3D::PIXELFORMAT_LIN_L32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_R32F:
    case GuestD3D::PIXELFORMAT_LIN_R32F:
        return D3DFMT_R32F;
    case GuestD3D::PIXELFORMAT_A16B16G16R16:
    case GuestD3D::PIXELFORMAT_LIN_A16B16G16R16:
        return D3DFMT_A16B16G16R16;
    case GuestD3D::PIXELFORMAT_Q16W16V16U16:
    case GuestD3D::PIXELFORMAT_LIN_Q16W16V16U16:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_A16B16G16R16F:
    case GuestD3D::PIXELFORMAT_LIN_A16B16G16R16F:
        return D3DFMT_A16B16G16R16F;
    case GuestD3D::PIXELFORMAT_A16B16G16R16F_EXPAND:
    case GuestD3D::PIXELFORMAT_LIN_A16B16G16R16F_EXPAND:
        return D3DFMT_UNKNOWN;

    // 64-bit formats
    case GuestD3D::PIXELFORMAT_A32L32:
    case GuestD3D::PIXELFORMAT_LIN_A32L32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_G32R32:
    case GuestD3D::PIXELFORMAT_LIN_G32R32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_V32U32:
    case GuestD3D::PIXELFORMAT_LIN_V32U32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_G32R32F:
    case GuestD3D::PIXELFORMAT_LIN_G32R32F:
        return D3DFMT_G32R32F;
    case GuestD3D::PIXELFORMAT_A32B32G32R32:
    case GuestD3D::PIXELFORMAT_LIN_A32B32G32R32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_Q32W32V32U32:
    case GuestD3D::PIXELFORMAT_LIN_Q32W32V32U32:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_A32B32G32R32F:
    case GuestD3D::PIXELFORMAT_LIN_A32B32G32R32F:
        return D3DFMT_UNKNOWN;

    // EDRAM / platform‐specific formats
    case GuestD3D::PIXELFORMAT_A2B10G10R10F_EDRAM:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_G16R16_EDRAM:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_A16B16G16R16_EDRAM:
        return D3DFMT_UNKNOWN;

    // Little-endian specific cases
    case GuestD3D::PIXELFORMAT_LE_X8R8G8B8:
        return D3DFMT_X8R8G8B8;
    case GuestD3D::PIXELFORMAT_LE_A8R8G8B8:
        return D3DFMT_A8R8G8B8;
    case GuestD3D::PIXELFORMAT_LE_X2R10G10B10:
        return D3DFMT_UNKNOWN;
    case GuestD3D::PIXELFORMAT_LE_A2R10G10B10:
        return D3DFMT_A2R10G10B10;

    // Special or unsupported types
    case GuestD3D::PIXELFORMAT_INDEX16:
    case GuestD3D::PIXELFORMAT_INDEX32:
    case GuestD3D::PIXELFORMAT_VERTEXDATA:
    case GuestD3D::PIXELFORMAT_NA:
        return D3DFMT_UNKNOWN;

    // Special variants of DXT formats
    case GuestD3D::PIXELFORMAT_DXT3A:
    case GuestD3D::PIXELFORMAT_LIN_DXT3A:
        return D3DFMT_DXT3;
    case GuestD3D::PIXELFORMAT_DXT3A_1111:
    case GuestD3D::PIXELFORMAT_LIN_DXT3A_1111:
        return D3DFMT_DXT3;
    case GuestD3D::PIXELFORMAT_DXT5A:
    case GuestD3D::PIXELFORMAT_LIN_DXT5A:
        return D3DFMT_DXT5;
    case GuestD3D::PIXELFORMAT_CTX1:
    case GuestD3D::PIXELFORMAT_LIN_CTX1:
        return D3DFMT_UNKNOWN;

    // Depth/stencil surface formats
    case GuestD3D::PIXELFORMAT_D24S8:
    case GuestD3D::PIXELFORMAT_LIN_D24S8:
        return D3DFMT_D24S8;
    case GuestD3D::PIXELFORMAT_D24X8:
    case GuestD3D::PIXELFORMAT_LIN_D24X8:
        return D3DFMT_D24X8;
    case GuestD3D::PIXELFORMAT_D24FS8:
    case GuestD3D::PIXELFORMAT_LIN_D24FS8:
        return D3DFMT_D24FS8;
    case GuestD3D::PIXELFORMAT_D32:
    case GuestD3D::PIXELFORMAT_LIN_D32:
        return D3DFMT_D32;

    default:
        assert(false);
        return D3DFMT_UNKNOWN;
    };
}

inline uint32_t GetBlend(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::D3DBLEND_ZERO:
        returnVal = D3DBLEND_ZERO;
        break;
    case GuestD3D::D3DBLEND_ONE:
        returnVal = D3DBLEND_ONE;
        break;
    case GuestD3D::D3DBLEND_SRCCOLOR:
        returnVal = D3DBLEND_SRCCOLOR;
        break;
    case GuestD3D::D3DBLEND_INVSRCCOLOR:
        returnVal = D3DBLEND_INVSRCCOLOR;
        break;
    case GuestD3D::D3DBLEND_SRCALPHA:
        returnVal = D3DBLEND_SRCALPHA;
        break;
    case GuestD3D::D3DBLEND_INVSRCALPHA:
        returnVal = D3DBLEND_INVSRCALPHA;
        break;
    case GuestD3D::D3DBLEND_DESTALPHA:
        returnVal = D3DBLEND_DESTALPHA;
        break;
    case GuestD3D::D3DBLEND_INVDESTALPHA:
        returnVal = D3DBLEND_INVDESTALPHA;
        break;
    case GuestD3D::D3DBLEND_DESTCOLOR:
        returnVal = D3DBLEND_DESTCOLOR;
        break;
    case GuestD3D::D3DBLEND_INVDESTCOLOR:
        returnVal = D3DBLEND_INVDESTCOLOR;
        break;
    case GuestD3D::D3DBLEND_SRCALPHASAT:
        returnVal = D3DBLEND_SRCALPHASAT;
        break;
    case GuestD3D::D3DBLEND_BLENDFACTOR:
        returnVal = D3DBLEND_BLENDFACTOR;
        break;
    case GuestD3D::D3DBLEND_INVBLENDFACTOR:
        returnVal = D3DBLEND_INVBLENDFACTOR;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetBlendOp(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::D3DBLENDOP_ADD:
        returnVal = D3DBLENDOP_ADD;
        break;
    case GuestD3D::D3DBLENDOP_SUBTRACT:
        returnVal = D3DBLENDOP_SUBTRACT;
        break;
    case GuestD3D::D3DBLENDOP_REVSUBTRACT:
        returnVal = D3DBLENDOP_REVSUBTRACT;
        break;
    case GuestD3D::D3DBLENDOP_MIN:
        returnVal = D3DBLENDOP_MIN;
        break;
    case GuestD3D::D3DBLENDOP_MAX:
        returnVal = D3DBLENDOP_MAX;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseBlendOp(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DBLENDOP_ADD:
        returnVal = GuestD3D::D3DBLENDOP_ADD;
        break;
    case D3DBLENDOP_SUBTRACT:
        returnVal = GuestD3D::D3DBLENDOP_SUBTRACT;
        break;
    case D3DBLENDOP_REVSUBTRACT:
        returnVal = GuestD3D::D3DBLENDOP_REVSUBTRACT;
        break;
    case D3DBLENDOP_MIN:
        returnVal = GuestD3D::D3DBLENDOP_MIN;
        break;
    case D3DBLENDOP_MAX:
        returnVal = GuestD3D::D3DBLENDOP_MAX;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseBlend(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DBLEND_ZERO:
        returnVal = GuestD3D::D3DBLEND_ZERO;
        break;
    case D3DBLEND_ONE:
        returnVal = GuestD3D::D3DBLEND_ONE;
        break;
    case D3DBLEND_SRCCOLOR:
        returnVal = GuestD3D::D3DBLEND_SRCCOLOR;
        break;
    case D3DBLEND_INVSRCCOLOR:
        returnVal = GuestD3D::D3DBLEND_INVSRCCOLOR;
        break;
    case D3DBLEND_SRCALPHA:
        returnVal = GuestD3D::D3DBLEND_SRCALPHA;
        break;
    case D3DBLEND_INVSRCALPHA:
        returnVal = GuestD3D::D3DBLEND_INVSRCALPHA;
        break;
    case D3DBLEND_DESTALPHA:
        returnVal = GuestD3D::D3DBLEND_DESTALPHA;
        break;
    case D3DBLEND_INVDESTALPHA:
        returnVal = GuestD3D::D3DBLEND_INVDESTALPHA;
        break;
    case D3DBLEND_DESTCOLOR:
        returnVal = GuestD3D::D3DBLEND_DESTCOLOR;
        break;
    case D3DBLEND_INVDESTCOLOR:
        returnVal = GuestD3D::D3DBLEND_INVDESTCOLOR;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseCmpFunc(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DCMP_NEVER:
        returnVal = GuestD3D::D3DCMP_NEVER;
        break;
    case D3DCMP_LESS:
        returnVal = GuestD3D::D3DCMP_LESS;
        break;
    case D3DCMP_EQUAL:
        returnVal = GuestD3D::D3DCMP_EQUAL;
        break;
    case D3DCMP_LESSEQUAL:
        returnVal = GuestD3D::D3DCMP_LESSEQUAL;
        break;
    case D3DCMP_GREATER:
        returnVal = GuestD3D::D3DCMP_GREATER;
        break;
    case D3DCMP_NOTEQUAL:
        returnVal = GuestD3D::D3DCMP_NOTEQUAL;
        break;
    case D3DCMP_GREATEREQUAL:
        returnVal = GuestD3D::D3DCMP_GREATEREQUAL;
        break;
    case D3DCMP_ALWAYS:
        returnVal = GuestD3D::D3DCMP_ALWAYS;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseStencilOp(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DSTENCILOP_KEEP:
        returnVal = GuestD3D::D3DSTENCILOP_KEEP;
        break;
    case D3DSTENCILOP_ZERO:
        returnVal = GuestD3D::D3DSTENCILOP_ZERO;
        break;
    case D3DSTENCILOP_REPLACE:
        returnVal = GuestD3D::D3DSTENCILOP_REPLACE;
        break;
    case D3DSTENCILOP_INCRSAT:
        returnVal = GuestD3D::D3DSTENCILOP_INCRSAT;
        break;
    case D3DSTENCILOP_DECRSAT:
        returnVal = GuestD3D::D3DSTENCILOP_DECRSAT;
        break;
    case D3DSTENCILOP_INVERT:
        returnVal = GuestD3D::D3DSTENCILOP_INVERT;
        break;
    case D3DSTENCILOP_INCR:
        returnVal = GuestD3D::D3DSTENCILOP_INCR;
        break;
    case D3DSTENCILOP_DECR:
        returnVal = GuestD3D::D3DSTENCILOP_DECR;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetCmpFunc(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::CmpFunc::D3DCMP_NEVER:
        returnVal = D3DCMP_NEVER;
        break;
    case GuestD3D::CmpFunc::D3DCMP_LESS:
        returnVal = D3DCMP_LESS;
        break;
    case GuestD3D::CmpFunc::D3DCMP_EQUAL:
        returnVal = D3DCMP_EQUAL;
        break;
    case GuestD3D::CmpFunc::D3DCMP_LESSEQUAL:
        returnVal = D3DCMP_LESSEQUAL;
        break;
    case GuestD3D::CmpFunc::D3DCMP_GREATER:
        returnVal = D3DCMP_GREATER;
        break;
    case GuestD3D::CmpFunc::D3DCMP_NOTEQUAL:
        returnVal = D3DCMP_NOTEQUAL;
        break;
    case GuestD3D::CmpFunc::D3DCMP_GREATEREQUAL:
        returnVal = D3DCMP_GREATEREQUAL;
        break;
    case GuestD3D::CmpFunc::D3DCMP_ALWAYS:
        returnVal = D3DCMP_ALWAYS;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetStencilOp(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::StencilOp::D3DSTENCILOP_KEEP:
        returnVal = D3DSTENCILOP_KEEP;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_ZERO:
        returnVal = D3DSTENCILOP_ZERO;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_REPLACE:
        returnVal = D3DSTENCILOP_REPLACE;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_INCRSAT:
        returnVal = D3DSTENCILOP_INCRSAT;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_DECRSAT:
        returnVal = D3DSTENCILOP_DECRSAT;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_INVERT:
        returnVal = D3DSTENCILOP_INVERT;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_INCR:
        returnVal = D3DSTENCILOP_INCR;
        break;
    case GuestD3D::StencilOp::D3DSTENCILOP_DECR:
        returnVal = D3DSTENCILOP_DECR;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetCullMode(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::D3DCULL_NONE:
        returnVal = D3DCULL_NONE;
        break;
    case GuestD3D::D3DCULL_CW:
        returnVal = D3DCULL_CW;
        break;
    case GuestD3D::D3DCULL_CCW:
        returnVal = D3DCULL_CCW;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetFillMode(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case GuestD3D::D3DFILL_POINT:
        returnVal = D3DFILL_POINT;
        break;
    case GuestD3D::D3DFILL_WIREFRAME:
        returnVal = D3DFILL_WIREFRAME;
        break;
    case GuestD3D::D3DFILL_SOLID:
        returnVal = D3DFILL_SOLID;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseFillMode(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DFILL_POINT:
        returnVal = GuestD3D::D3DFILL_POINT;
        break;
    case D3DFILL_WIREFRAME:
        returnVal = GuestD3D::D3DFILL_WIREFRAME;
        break;
    case D3DFILL_SOLID:
        returnVal = GuestD3D::D3DFILL_SOLID;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline uint32_t GetReverseCullMode(uint32_t p_value)
{
    uint32_t returnVal;

    switch (p_value)
    {
    case D3DCULL_NONE:
        returnVal = GuestD3D::D3DCULL_NONE;
        break;
    case D3DCULL_CW:
        returnVal = GuestD3D::D3DCULL_CW;
        break;
    case D3DCULL_CCW:
        returnVal = GuestD3D::D3DCULL_CCW;
        break;
    default:
        assert(false);
        break;
    }

    return returnVal;
}

inline D3DDECLTYPE ConvertGuestDeclType(uint32_t guestType)
{
    switch (guestType)
    {
    case GuestD3D::D3DDECLTYPE_FLOAT1:
        return D3DDECLTYPE_FLOAT1;

    case GuestD3D::D3DDECLTYPE_FLOAT2:
        return D3DDECLTYPE_FLOAT2;

    case GuestD3D::D3DDECLTYPE_FLOAT3:
        return D3DDECLTYPE_FLOAT3;

    case GuestD3D::D3DDECLTYPE_FLOAT4:
        return D3DDECLTYPE_FLOAT4;

    case GuestD3D::D3DDECLTYPE_D3DCOLOR:
        return D3DDECLTYPE_D3DCOLOR;

    case GuestD3D::D3DDECLTYPE_UBYTE4:
        return D3DDECLTYPE_UBYTE4;

    case GuestD3D::D3DDECLTYPE_UBYTE4N:
        return D3DDECLTYPE_UBYTE4N;

    case GuestD3D::D3DDECLTYPE_SHORT2:
        return D3DDECLTYPE_SHORT2;

    case GuestD3D::D3DDECLTYPE_SHORT4:
        return D3DDECLTYPE_SHORT4;

    case GuestD3D::D3DDECLTYPE_SHORT2N:
        return D3DDECLTYPE_SHORT2N;

    case GuestD3D::D3DDECLTYPE_SHORT4N:
        return D3DDECLTYPE_SHORT4N;

    case GuestD3D::D3DDECLTYPE_HEND3N:
        return D3DDECLTYPE_DEC3N; // not a perfect match

    case GuestD3D::D3DDECLTYPE_DEC3N:
        return D3DDECLTYPE_DEC3N;

    case GuestD3D::D3DDECLTYPE_FLOAT16_2:
        return D3DDECLTYPE_FLOAT16_2;

    case GuestD3D::D3DDECLTYPE_FLOAT16_4:
        return D3DDECLTYPE_FLOAT16_4;

    case GuestD3D::D3DDECLTYPE_UNUSED:
        return D3DDECLTYPE_UNUSED;

    default:
        Log::Error("VertexDecl", "Unknown guest type: ", guestType);
        DebugBreak();
        return D3DDECLTYPE_UNUSED;
    }
}

inline D3DPRIMITIVETYPE ConvertGuestPrimType(GuestD3D::PrimitiveType guestType)
{
    switch (guestType)
    {
    case GuestD3D::XD3DPT_POINTLIST:
        return D3DPT_POINTLIST;
    case GuestD3D::XD3DPT_LINELIST:
        return D3DPT_LINELIST;
    case GuestD3D::XD3DPT_LINESTRIP:
        return D3DPT_LINESTRIP;
    case GuestD3D::XD3DPT_TRIANGLELIST:
        return D3DPT_TRIANGLELIST;
    case GuestD3D::XD3DPT_TRIANGLEFAN:
        return D3DPT_TRIANGLEFAN;
    case GuestD3D::XD3DPT_TRIANGLESTRIP:
        return D3DPT_TRIANGLESTRIP;
    case GuestD3D::XD3DPT_QUADLIST:
        return D3DPT_TRIANGLELIST;
    default:
        Log::Error("ConvertGuestPrimType", "Unknown primitive type: ", guestType);
        DebugBreak();
        return D3DPT_FORCE_DWORD;
    }
}

inline int GetPrimitiveCount(int vertexCount, GuestD3D::PrimitiveType primType)
{
    switch (primType)
    {
    case GuestD3D::XD3DPT_POINTLIST:
        return vertexCount;

    case GuestD3D::XD3DPT_LINELIST:
        return vertexCount / 2;

    case GuestD3D::XD3DPT_LINESTRIP:
        return (vertexCount > 1) ? (vertexCount - 1) : 0;

    case GuestD3D::XD3DPT_TRIANGLELIST:
        return vertexCount / 3;

    case GuestD3D::XD3DPT_TRIANGLEFAN:
        return (vertexCount > 2) ? (vertexCount - 2) : 0;

    case GuestD3D::XD3DPT_TRIANGLESTRIP:
        return (vertexCount > 2) ? (vertexCount - 2) : 0;

    case GuestD3D::XD3DPT_RECTLIST:
        return vertexCount / 4;

    case GuestD3D::XD3DPT_QUADLIST:
        return vertexCount / 4;

    default:
        Log::Error("GetPrimitiveCount", "Unknown primitive type: ", primType);
        DebugBreak();
        return 0;
    }
}

struct GuestResource
{
    be<uint32_t> Common;
    be<uint32_t> ReferenceCount;
    be<uint32_t> Fence;
    be<uint32_t> ReadFence;
    be<uint32_t> Identifier;
    be<uint32_t> BaseFlush;
};

struct GuestBaseTexture : GuestResource
{
    be<uint32_t> MipFlush;
    GuestD3D::GPUTEXTURE_FETCH_CONSTANT Format;

    uint32_t GetLevelCount()
    {
        return ((Format.dword[4] >> 6) & 0xF) + 1;
    }
};

struct ALIGN(8) GuestVertexDeclaration : GuestResource
{
    be<uint32_t> m_count;
    be<uint32_t> m_maxStream;

    union
    {
        uint8_t m_streamMask[16];
        uint64_t m_streamMask64[2];
    };

    be<uint32_t> m_uniqueness;
    GuestD3D::VertexElement m_element[];

    void PrintDeclaration()
    {
        if (m_count == 0)
        {
            Log::Info("VertexDecl", "Declaration is empty, count == 0");
            return;
        }

        for (uint32_t i = 0; i < m_count.get(); ++i)
        {
            const auto &elem = m_element[i];

            Log::Info("VertexDecl", "Element [", i, "] | Stream: ", elem.Stream,
                      " Offset: ", elem.Offset,
                      " Type: ", GuestD3D::GetDeclTypeName(elem.Type),
                      " Method: ", GuestD3D::GetDeclMethodName(elem.Method),
                      " Usage: ", GuestD3D::GetDeclUsageName(elem.Usage));
        }
    }
};

struct GuestViewport
{
    be<float> x;
    be<float> y;
    be<float> width;
    be<float> height;
    be<float> minZ;
    be<float> maxZ;
};

struct GuestDeviceBase
{
    GuestD3D::TagCollection m_pending;

    be<uint64_t> m_predicated_PendingMask2;
    be<uint32_t> m_pRing;
    be<uint32_t> m_pRingLimit;
    be<uint32_t> m_pRingGuarantee;
    be<uint32_t> m_referenceCount;

    be<uint32_t> m_setRenderStateFunctions[101];
    be<uint32_t> m_setSamplerStateFunctions[20];

    be<uint32_t> m_getRenderStateCall[101];
    be<uint32_t> m_getSamplerStateCall[20];

    ALIGN(128)
    GuestD3D::Constants m_constants;

    float m_clipPlanes[6][4];

    PADDING(512, 1);
};

struct ALIGN(64) GuestDevice : GuestDeviceBase
{
    PADDING(940, 2);

    be<uint32_t> m_vertexDeclaration;

    PADDING(824, 3);

    GuestViewport m_viewport;

    PADDING(11284, 4);
};

struct Test
{
    void Testt()
    {
        sizeof(GuestDevice);
    }
};

static_assert(sizeof(GuestDeviceBase) == 0x2A80);
static_assert(sizeof(GuestDevice) == 0x5E00);
static_assert(sizeof(GuestVertexDeclaration) == 0x38);
static_assert(sizeof(GuestD3D::Constants) == 0x23A0);

static_assert(offsetof(GuestDevice, m_vertexDeclaration) == 0x2E2C);
static_assert(offsetof(GuestDevice, m_constants) == 0x480);
static_assert(offsetof(GuestDevice, m_viewport) == 0x3168);
