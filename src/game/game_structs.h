#pragma once

#include "xex.h"

namespace renderengine
{
    enum Type : uint32_t
    {
        TYPE_NA = 0xFFFFFFFF,
        TYPE_VERTEX = 0x0,
        TYPE_PIXEL = 0x1,
        TYPE_FORCEENUMSIZEINT = 0x7FFFFFFF,
    };

    struct Parts
    {
        uint32_t m_unused1;
        unsigned int m_unused2;
        uint32_t m_unused3;
        unsigned int m_unused4;
        uint32_t m_unused5;
        unsigned int m_unused6;
        unsigned int m_unused7;
    };

    struct ProgBuffer_Parameters
    {
        be<uint32_t> m_buffer;
        be<Type> m_type;
        be<unsigned int> m_size;
        Parts m_parts;
    };

    struct ProgramBuffer
    {
        be<Type> m_type;                // 0
        be<unsigned __int16> m_unused1; // 4
        be<unsigned __int16> m_unused2; // 6
        be<unsigned int> m_unused3;     // 8
        be<unsigned int> m_unused4;     // 12
        be<uint32_t> m_part;            // 16

        // When D3D::CreatePixelShader is called, the game passes in ProgramBuffer + 20 as the shader object.
        uint32_t m_pixelShaderID; // 20
    };

    struct ProgramVariableHandle
    {
        be<unsigned __int8> m_index;
        be<unsigned __int8> m_dataType;
        be<unsigned __int8> m_programType;
        be<unsigned __int8> m_numConstants;
    };

    struct D3DResource
    {
        unsigned int Common;
        unsigned int ReferenceCount;
        unsigned int Fence;
        unsigned int ReadFence;
        be<unsigned int> Identifier;
        unsigned int BaseFlush;
    };

    union GPUTEXTURE_FETCH_CONSTANT
    {
        unsigned int dword[6];
    };

    struct D3DBaseTexture : D3DResource
    {
        unsigned int MipFlush;
        GPUTEXTURE_FETCH_CONSTANT Format;
    };

    struct Texture : D3DBaseTexture
    {
    };

    struct FloatShaderStateIterator
    {
        be<uint32_t> m_dest;   // Matrix4x4
        be<uint32_t> m_handle; // ProgramVariableHandle
    };
}

namespace rw
{
    template <int N>
    struct BaseResources
    {
        uint32_t m_baseResources[N];
    };

    struct Resource : BaseResources<5>
    {
    };
}