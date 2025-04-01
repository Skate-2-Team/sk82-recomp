#pragma once

#include <SDL3/SDL.h>
#include <winerror.h>
#include <d3d9.h>

#include "log.h"
#include "ppc/ppc_recomp_shared.h"

class Video
{
public:
    const int m_screenWidth = 1280;
    const int m_screenHeight = 720;
    const char *m_windowTitle = "Skate 2 - Recompilation";

    SDL_Window *m_window = nullptr;
    HWND m_hwnd = nullptr;
    LPDIRECT3D9 m_d3d = nullptr;
    LPDIRECT3DDEVICE9 m_d3dDevice = nullptr;

    bool m_quit = false;

    Video();

    bool MakeWindow();
    bool InitD3D();

    void SampleRenderFrame();
    void WindowLoop();
};

inline std::shared_ptr<Video> g_video = nullptr;
