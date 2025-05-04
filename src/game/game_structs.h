#pragma once

#include "graphics/d3d_context.h"

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
        be<uint32_t> m_unused2;
        uint32_t m_unused3;
        be<uint32_t> m_unused4;
        uint32_t m_unused5;
        be<uint32_t> m_unused6;
        be<uint32_t> m_unused7;
    };

    struct ProgBuffer_Parameters
    {
        be<uint32_t> m_buffer;
        be<Type> m_type;
        be<uint32_t> m_size;
        Parts m_parts;
    };

    struct ProgramBuffer
    {
        be<Type> m_type;        // 0
        be<uint16_t> m_unused1; // 4
        be<uint16_t> m_unused2; // 6
        be<uint32_t> m_unused3; // 8
        be<uint32_t> m_unused4; // 12
        be<uint32_t> m_part;    // 16

        // When D3D::CreatePixelShader is called, the game passes in ProgramBuffer + 20 as the shader object.
        uint32_t m_pixelShaderID; // 20
    };

    struct ProgramVariableHandle
    {
        be<uint8_t> m_index;
        be<uint8_t> m_dataType;
        be<uint8_t> m_programType;
        be<uint8_t> m_numConstants;
    };

    struct FloatShaderStateIterator
    {
        be<uint32_t> m_dest;   // Matrix4x4
        be<uint32_t> m_handle; // ProgramVariableHandle
    };

    struct VertexDescriptor
    {
        be<uint32_t> m_d3dVertexDeclaration;
        be<uint32_t> m_typesFlags;
        be<uint16_t> m_numElements;
        short m_refCount;
        be<uint16_t> m_instanceStreams;
        be<uint16_t> m_pad0;
    };

    struct RasterizerState
    {
        be<uint32_t> m_fillMode;
        be<uint32_t> m_cullMode;
        be<uint32_t> m_depthBias;
        be<uint32_t> m_slopeScaleDepthBias;
        be<uint32_t> m_multiSampleMask;
        be<uint32_t> m_scissorTestEnable;
        be<uint32_t> m_multiSampleAntiAlias;
        be<uint32_t> m_unkInt8;
        be<uint32_t> m_viewportEnable;
        be<uint32_t> m_halfPixelOffset;
        be<uint32_t> m_primitiveResetEnable;
        be<uint32_t> m_primitiveResetIndex;
    };

    struct SamplerState
    {
        be<float> m_mipmaplodBias;
        be<float> m_anisotropyBias;

        union SamplerStatePack
        {
            struct
            {
                uint8_t m_addressU;
                uint8_t m_addressV;
                uint8_t m_addressW;
                uint8_t m_clamppolicy;
                uint8_t m_mipfilter;
                char m_hgradientexpbias;
                char m_vgradientexpbias;
                uint8_t m_borderColor;
                uint8_t m_forcebcwtomax;
                uint8_t m_trilinearthreshold;
                uint8_t m_minfilter;
                uint8_t m_minfilterz;
                uint8_t m_magfilter;
                uint8_t m_magfilterz;
                uint8_t m_separatezfilterenable;
                uint8_t m_maxanisotropy;
                uint8_t m_maxmiplevel;
                uint8_t m_minmiplevel;
                uint8_t m_srgbtexture;
                uint8_t m_pad;
            };

            uint32_t m_dword[5];
        } m_samplerStatePack;

        SamplerStatePack GetSwappedPack()
        {
            SamplerStatePack swappedPack = {};

            swappedPack.m_dword[0] = ByteSwap(m_samplerStatePack.m_dword[0]);
            swappedPack.m_dword[1] = ByteSwap(m_samplerStatePack.m_dword[1]);
            swappedPack.m_dword[2] = ByteSwap(m_samplerStatePack.m_dword[2]);
            swappedPack.m_dword[3] = ByteSwap(m_samplerStatePack.m_dword[3]);
            swappedPack.m_dword[4] = ByteSwap(m_samplerStatePack.m_dword[4]);

            return swappedPack;
        }
    };

    struct BlendStatePack
    {
        GuestD3D::BlendState m_blendStates[4];
        be<uint32_t> m_colorWriteEnable;
        be<uint32_t> m_colorWriteEnable1;
        be<uint32_t> m_colorWriteEnable2;
        be<uint32_t> m_colorWriteEnable3;
        be<uint32_t> m_alphaToMaskOffsets;
        be<uint32_t> m_blendFactor;
        be<uint32_t> m_alphaToMaskEnable;
        be<uint32_t> m_highPrecisionBlendEnable;
        be<uint32_t> m_highPrecisionBlendEnable1;
        be<uint32_t> m_highPrecisionBlendEnable2;
        be<uint32_t> m_highPrecisionBlendEnable3;
        be<uint32_t> m_alphaFunc;
        be<uint32_t> m_alphaTestEnable;
        be<uint32_t> m_alphaRef;
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