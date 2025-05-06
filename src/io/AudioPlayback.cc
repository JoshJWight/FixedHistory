#include "AudioPlayback.hh"

#include <iostream>
#include <stdexcept>
#include <functional>

AudioPlayback::AudioPlayback()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL initialization failed");
    }
}

void AudioPlayback::init(const std::string& audioFilePath)
{
     // Load the WAV file
    Uint32 wavLength;
    
    std::cout << "Attempting to load WAV file: " << audioFilePath << std::endl;
    
    if (SDL_LoadWAV(audioFilePath.c_str(), &m_wavSpec, &m_wavBuffer, &wavLength) == NULL) {
        std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("Failed to load WAV file");
    }

    std::cout << "WAV file loaded successfully" << std::endl;
    std::cout << "Format: " << m_wavSpec.format << std::endl;
    std::cout << "Channels: " << (int)m_wavSpec.channels << std::endl;
    std::cout << "Sample Rate: " << m_wavSpec.freq << std::endl;
    std::cout << "Length: " << wavLength << " bytes" << std::endl;

    // Set up the audio data structure
    
    m_audio.buffer = m_wavBuffer;    // Store the start of the buffer
    m_audio.position = m_wavBuffer;  // Current position starts at the beginning
    m_audio.length = wavLength;
    m_audio.finished = false;

    // Set up the callback
    m_wavSpec.callback = AudioPlayback::audioCallback;
    m_wavSpec.userdata = &m_audio;

    // Open audio device
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &m_wavSpec, &m_deviceSpec, 0);
    if (deviceId == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        SDL_FreeWAV(m_wavBuffer);
        SDL_Quit();
        throw std::runtime_error("Failed to open audio device");
    }

    std::cout << "Audio device opened successfully" << std::endl;

    // Calculate total duration for progress display
    float bytesPerSample = SDL_AUDIO_BITSIZE(m_wavSpec.format) / 8.0f;
    float samplesPerChannel = wavLength / (bytesPerSample * m_wavSpec.channels);
    Uint32 totalDurationMs = static_cast<Uint32>((samplesPerChannel * 1000.0f) / m_wavSpec.freq);
    std::cout << "Total duration: " << totalDurationMs / 1000 << " seconds" << std::endl;

    // Start playing
    SDL_PauseAudioDevice(deviceId, 0);
}

void AudioPlayback::update(const AudioContext& context) {
    m_lastContext = context;
}

void AudioPlayback::audioCallback(void* userdata, Uint8* stream, int streamLength) {
    AudioData* audio = static_cast<AudioData*>(userdata);
    
    if (audio->finished) {
        // Fill buffer with silence if we're done
        SDL_memset(stream, 0, streamLength);
        return;
    }

    // Calculate how many bytes to copy
    Uint32 remainingLength = audio->length - (audio->position - audio->buffer);
    Uint32 copyLength = (remainingLength < static_cast<Uint32>(streamLength)) ? remainingLength : streamLength;

    // Copy audio data to stream
    if (copyLength > 0) {
        SDL_memcpy(stream, audio->position, copyLength);
        audio->position += copyLength;

        // Fill the rest with silence if we didn't have enough data
        if (copyLength < static_cast<Uint32>(streamLength)) {
            SDL_memset(stream + copyLength, 0, streamLength - copyLength);
            audio->finished = true;
        }
    } else {
        SDL_memset(stream, 0, streamLength);
        audio->finished = true;
    }
}