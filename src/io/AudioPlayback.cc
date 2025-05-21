#include "AudioPlayback.hh"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <bitset>

AudioPlayback::AudioPlayback()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL initialization failed");
    }
}

void AudioPlayback::init(const std::string& audioFilePath)
{
    std::cout << "Attempting to load WAV file: " << audioFilePath << std::endl;
    
    if (SDL_LoadWAV(audioFilePath.c_str(), &m_wavSpec, &m_wavBuffer, &m_bufferSize) == NULL) {
        std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("Failed to load WAV file");
    }

    std::cout << "WAV file loaded successfully" << std::endl;
    std::cout << "Format: 0b" << std::bitset<16>(m_wavSpec.format) << std::endl;
    std::cout << "Channels: " << (int)m_wavSpec.channels << std::endl;
    std::cout << "Sample Rate: " << m_wavSpec.freq << std::endl;
    std::cout << "Length: " << m_bufferSize << " bytes" << std::endl;

    short expectedFormat = 32784; // 16-bit signed integer, little-endian
    if(!(m_wavSpec.format == expectedFormat)) {
        throw std::runtime_error("WAV format different than expected!");
    }

    // Set up the callback
    m_wavSpec.callback = AudioPlayback::audioCallback;
    m_wavSpec.userdata = this;

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
    std::cout << "Bytes per sample: " << bytesPerSample << std::endl;
    float samplesPerChannel = m_bufferSize / (bytesPerSample * m_wavSpec.channels);
    Uint32 totalDurationMs = static_cast<Uint32>((samplesPerChannel * 1000.0f) / m_wavSpec.freq);
    std::cout << "Total duration: " << totalDurationMs / 1000 << " seconds" << std::endl;

    // Start playing
    SDL_PauseAudioDevice(deviceId, 0);
    m_currentPosition = 0;
}

void AudioPlayback::update(const AudioContext& context) {
    //TODO this probably needs to be mutex-protected
    m_lastContext = context;
}

void AudioPlayback::audioCallback(void* userdata, Uint8* stream, int streamLength) {
    AudioPlayback* audioPlayback = static_cast<AudioPlayback*>(userdata);
    audioPlayback->doAudioCallback(stream, streamLength);
}

void AudioPlayback::doAudioCallback(Uint8* stream, int streamLength) {
    float expectedPosition = static_cast<float>(m_lastContext.tick) / m_lastContext.frameRate * static_cast<float>(m_wavSpec.freq);

    if(m_lastContext.playbackSpeed == 0)
    {
        //Paused
        //TODO might be able to do something interesting with FFT
        SDL_memset(stream, 0, streamLength);
        return;
    }
    else if(m_lastContext.playbackSpeed == 1)
    {
        //Normal playback
        if(m_lastContext.backwards) {
            if(m_currentPosition < streamLength) {
                SDL_memset(stream, 0, streamLength);
                return;
            }

            for(int i = 0; i < streamLength; ++i) {
                stream[i] = m_wavBuffer[m_currentPosition - i];
            }

            m_currentPosition -= streamLength;
        }
        else{
            if(m_currentPosition + streamLength > m_bufferSize) {
                SDL_memset(stream, 0, streamLength);
                return;
            }

            SDL_memcpy(stream, m_wavBuffer + m_currentPosition, streamLength);
            m_currentPosition += streamLength;
        }
    }
    else
    {
        //Custom playback speed
        float position = m_currentPosition;
        for(int i = 0; i < streamLength; ++i) {

            int floor = std::floor(position);
            int ceil = std::ceil(position);
            if(floor == ceil) {
                stream[i] = m_wavBuffer[floor];
            }
            else {
                float fraction = position - floor;
                stream[i] = static_cast<Uint8>((1.0f - fraction) * m_wavBuffer[floor] + fraction * m_wavBuffer[ceil]);
            }

            if(m_lastContext.backwards) {
                position -= m_lastContext.playbackSpeed;
            }
            else {
                position += m_lastContext.playbackSpeed;
            }
        }
        m_currentPosition = static_cast<size_t>(position);
    }


    
}