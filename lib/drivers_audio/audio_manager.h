#pragma once

#define HELIX_LOG_LEVEL LogLevelHelix::Warning

#include <Arduino.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "AudioTools/CoreAudio/AudioHttp/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioLibs/MemoryManager.h"
#include "AudioTools/CoreAudio/AudioMetaData/MetaDataFilter.h"
#include "AudioTools/Disk/AudioSourceLittleFS.h"
#include "LittleFS.h"
#include "audio_source_littlefs_mounted.h"
#include "event_publisher.h"
#include "events.h"

namespace dict {

class EncodedAudioStreamPatch : public EncodedAudioStream {
// we use this patch class to modify the protected enc_out. once the issue is resovled, we can remove this class.
// https://github.com/pschatzmann/arduino-audio-tools/issues/2165
public:
    EncodedAudioStreamPatch(AudioBoardStream& out, MP3DecoderHelix& mp3) : EncodedAudioStream(&out, &mp3) {}
    bool begin(AudioInfo info) {
        setAudioInfo(info);
        return begin();
    }
    
    bool begin() override {
        setupReader();
        ReformatBaseStream::begin();
        LOGD("Fix the bug: Here info has sample_rate %d, audioInfo() has sample_rate %d, but enc_out.info has sample_rate %d.", info.sample_rate, audioInfo().sample_rate, enc_out.audioInfo().sample_rate);
        return enc_out.begin(info);
      }
};

class AudioManager {
public:
    // Constructor/Destructor
    AudioManager();
    ~AudioManager();
    
    // Core lifecycle methods
    bool initialize(); // Initialize audio system and ES8311 codec
    void shutdown(); // Clean shutdown of audio system and free resources
    void tick(); // Process audio events and state updates
    bool isReady() const; // Check if audio system is ready for playback
    
    // Audio playback methods
    bool play(const char* url); // Play audio from URL or local file path
    bool stop(); // Stop current audio playback
    
    // Utility/getter methods
    bool isCurrentlyPlaying() const { return isPlaying && !isPaused; } // Check if audio is currently playing
    bool isCurrentlyPaused() const { return isPaused; } // Check if audio is paused
    String getCurrentUrl() const { return currentUrl; } // Get current audio URL/file path
    void setVolume(float volume); // Set audio volume (0.0 to 1.0)

private:
    // Audio board and output (ES8311)
    AudioBoard board;
    AudioBoardStream out;
    AudioInfo info;
    
    // Network stream and MP3 decoder pipeline
    MP3DecoderHelix mp3;
    EncodedAudioStreamPatch decoded;
    MetaDataFilter metadataFilter;
    URLStream *url;
    File *fileStream;
    StreamCopy *copier;
    
    // AudioPlayer for local files
    AudioSourceLittleFSMounted *localFileSource;
    AudioPlayer *localFilePlayer;
    
    // Task handle for audio processing
    TaskHandle_t audioTaskHandle;
    
    // State management
    bool isInitialized;
    bool isPlaying;
    bool isPaused;
    String currentUrl;
    SemaphoreHandle_t audioMutex;
    
    // Timeout settings
    uint32_t playbackCompleteCountThreshold;
    
    uint32_t expectedContentLength;

    // Private methods
    static void audioTask(void* parameter);
    void processAudio();

};

} // namespace dict
