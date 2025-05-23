#include "video.h"

Video::Video()
{
    Log::Info("Video", "Video constructor called.");
}

bool Video::MakeWindow()
{
    Log::Info("Video", "Creating window...");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Log::Error("Video", "SDL could not initialize: ", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(m_windowTitle, m_screenWidth, m_screenHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        Log::Error("Video", "Window could not be created: ", SDL_GetError());
        SDL_Quit();
        return false;
    }

    m_hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

    SDL_ShowWindow(m_window);

    return true;
}

bool Video::InitD3D()
{
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d)
    {
        Log::Error("Video", "Failed to create D3D object.");
        return false;
    }

    D3DPRESENT_PARAMETERS d3dpp{};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3dpp.hDeviceWindow = m_hwnd;

    if (FAILED(m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &m_d3dDevice)))
    {
        Log::Error("Video", "Failed to create D3D device.");
        return false;
    }

    InitFPSCounter();

    return true;
}

void Video::InitFPSCounter()
{
    m_lastTime = std::chrono::steady_clock::now();
    m_frameCount = 0;
}

void Video::UpdateFPSCounter()
{
    ++m_frameCount;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTime).count();

    if (elapsed >= 1000)
    {
        // compute FPS as frames per second
        m_currentFps = static_cast<double>(m_frameCount) * 1000.0 / elapsed;

        // update window title with FPS
        std::string title = std::string(m_windowTitle) + " - FPS: " + std::to_string(static_cast<int>(m_currentFps));

        // draw FPS on screen using d3d device
        Log::Info("Video", "FPS: ", m_currentFps);

        // reset
        m_frameCount = 0;
        m_lastTime = now;
    }
}

// Just for debugs
void Video::SampleRenderFrame()
{
    m_d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(30, 30, 30), 1.0f, 0);

    if (SUCCEEDED(m_d3dDevice->BeginScene()))
    {
        struct CUSTOMVERTEX
        {
            FLOAT x, y, z, rhw;
            DWORD color;
        };

        // Create a rectangle from (200, 150) to (600, 450)
        CUSTOMVERTEX vertices[] =
            {
                {200.0f, 150.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0)},
                {600.0f, 150.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0)},
                {200.0f, 450.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0)},
                {600.0f, 450.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0)}};

        // Draw the rectangle using a triangle strip
        m_d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        m_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CUSTOMVERTEX));

        m_d3dDevice->EndScene();
    }
}
