#ifndef AUDIO_PLAYBACK_HH
#define AUDIO_PLAYBACK_HH

#include <string>
#include <SDL2/SDL.h>

// Audio callback structure to keep track of playback position
struct AudioData {
    Uint8* buffer;     // Start of the buffer
    Uint8* position;   // Current position in the buffer
    Uint32 length;     // Total length of the buffer
    bool finished;     // Flag to indicate if playback is complete
};

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
        SDL_FreeWAV(m_wavBuffer);
        SDL_Quit();
    }

private:
    static void audioCallback(void* userdata, Uint8* stream, int streamLength);

    SDL_AudioSpec m_wavSpec;
    SDL_AudioSpec m_deviceSpec;
    Uint8* m_wavBuffer;
    AudioData m_audio;

    AudioContext m_lastContext;
};

#endif