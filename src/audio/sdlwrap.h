#pragma once
#include <SDL3/SDL.h>
#include "log.h"

class SdlAudio
{
    SDL_AudioStream *m_audioStream{};
    SDL_AudioSpec m_audioSpec{};
    SDL_AudioDeviceID m_audioDeviceId{};

    static void InitSDL()
    {
        if (!SDL_Init(SDL_INIT_AUDIO))
        {
            Log::Error("SdlAudio::InitSDL", "SDL failed to init");
            __debugbreak();
        }
    }
    void OpenStream()
    {
        m_audioStream = SDL_OpenAudioDeviceStream(m_audioDeviceId, &m_audioSpec, nullptr, nullptr);
        if (!m_audioStream)
        {
            Log::Error("SdlAudio::OpenStream", "SDL failed to open device");
            __debugbreak();
        }
        SDL_ResumeAudioStreamDevice(m_audioStream);
    }
public:
    SDL_AudioStream  *GetStream()        { return m_audioStream; }
    SDL_AudioSpec     GetAudioSpec()     { return m_audioSpec; }
    SDL_AudioDeviceID GetAudioDeviceId() { return m_audioDeviceId; }

    void PutBuffer(void const* data, size_t const size) const
    {
        if (m_audioStream)
        {
            SDL_PutAudioStreamData(m_audioStream, data, size);
        }
    }

    SdlAudio() = delete;
    explicit SdlAudio(SDL_AudioSpec const& spec, SDL_AudioDeviceID deviceId)
        : m_audioSpec(spec), m_audioDeviceId(deviceId)
    {
        InitSDL();
        OpenStream();
    }
};