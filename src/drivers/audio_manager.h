#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

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
#include "core/driver_interface.h"

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
        // is_output_notify = false;
        setupReader();
        ReformatBaseStream::begin();
        LOGD("Fix the bug: Here info has sample_rate %d, audioInfo() has sample_rate %d, but enc_out.info has sample_rate %d.", info.sample_rate, audioInfo().sample_rate, enc_out.audioInfo().sample_rate);
        return enc_out.begin(info);
      }
};

class AudioManager : public DriverInterface {
private:
    // Audio board and output (ES8311)
    AudioBoard board;
    AudioBoardStream out;
    AudioInfo info;
    
    // Network stream and MP3 decoder pipeline
    MP3DecoderHelix mp3;
    EncodedAudioStreamPatch decoded;
    URLStream *url;
    StreamCopy *copier;
    
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
    void fixRemoveID3Info(Stream& s);
    static void audioTask(void* parameter);
    void processAudio();
    
public:
    AudioManager();
    ~AudioManager();
    
    // DriverInterface implementation
    bool initialize() override;
    void shutdown() override;
    void tick() override;
    bool isReady() const override;
    
    
    // Audio playback methods
    bool play(const char* url);
    bool stop();
    
    // Utility methods
    bool isCurrentlyPlaying() const { return isPlaying && !isPaused; }
    bool isCurrentlyPaused() const { return isPaused; }
    String getCurrentUrl() const { return currentUrl; }
    void setVolume(float volume);

};

#endif // AUDIO_MANAGER_H
