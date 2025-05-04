#pragma once

#include "video_hooks.h"
#include "game/game_structs.h"
#include <xxhash.h>

namespace Shaders
{
    enum XenosShaderType : uint32_t
    {
        XPIXEL_SHADER = 0x102A1100,
        XVERTEX_SHADER = 0x102A1101,
    };

    struct XenosShaderHeader
    {
        XenosShaderType m_type;
        byte m_pad[36];
        be<uint32_t> m_updbPathSize;
        char m_updbPath[];
    };

    inline std::atomic<bool> g_isShaderLoaded = false;
    inline int g_shaderErrorCount = 0;

    inline IDirect3DVertexShader9 *g_pVertexShader = nullptr;

    inline IDirect3DVertexShader9 *g_passThroughVs = nullptr;
    inline IDirect3DPixelShader9 *g_passThroughPs = nullptr;

    inline int g_curShaderKey = 0;

    inline std::map<uint32_t, IDirect3DPixelShader9 *> g_pPixelShaders;
    inline std::map<uint32_t, std::string> g_pPixelShaderNames;

    void PrecompileShaders();
}