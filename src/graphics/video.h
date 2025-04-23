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

    std::chrono::steady_clock::time_point m_lastTime;
    uint32_t m_frameCount = 0;
    float m_currentFps = 0.0f;

    Video();

    bool MakeWindow();
    bool InitD3D();

    void SampleRenderFrame();
    void WindowLoop();

    void InitFPSCounter();
    void UpdateFPSCounter();

    inline void PresentFrame()
    {
        HRESULT hr = m_d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        if (hr == D3DERR_DEVICELOST)
        {
            Log::Error("Video", "Device lost, trying to reset...");
        }
    }

    inline void EventFlush()
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                m_quit = true;
            }
        }
    }
};

inline std::shared_ptr<Video> g_video = nullptr;
