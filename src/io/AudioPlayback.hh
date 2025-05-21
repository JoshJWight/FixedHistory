#ifndef AUDIO_PLAYBACK_HH
#define AUDIO_PLAYBACK_HH

#include <string>
#include <SDL2/SDL.h>

struct AudioContext {
    float frameRate;
    float playbackSpeed;
    int tick;
    bool backwards;
};

class AudioPlayback {

public:
    AudioPlayback();
    void init(const std::string& audioFilePath);

    void update(const AudioContext& context);

    ~AudioPlayback() {
        SDL_FreeWAV((Uint8*)m_wavBuffer);
        SDL_Quit();
    }

private:
    static void audioCallback(void* userdata, Uint8* stream, int streamLength);
    void doAudioCallback(Uint8* stream, int streamLength);

    SDL_AudioSpec m_wavSpec;
    SDL_AudioSpec m_deviceSpec;
    short* m_wavBuffer;
    uint32_t m_bufferSize;
    size_t m_currentPosition;

    AudioContext m_lastContext;
};

#endif