// Enable Helix logging at Debug level
// #define HELIX_LOG_LEVEL LogLevelHelix::Debug

#include "audio_manager.h"
#include "esp_log.h"

static const char *TAG = "AudioManager";
// Route allocations > 256 bytes to PSRAM if available
static MemoryManager s_audioMem(256);

AudioManager::AudioManager()
    : board(AudioDriverES8311, NoPins), out(board), info(32000, 2, 16), url(nullptr), mp3(), decoded(out, mp3), copier(nullptr),
      audioTaskHandle(nullptr), isInitialized(false), isPlaying(false), currentUrl(""), 
      playbackCompleteCountThreshold(1000) {}

AudioManager::~AudioManager() {
    end();
    if (url) {
        delete url;
        url = nullptr;
    }
    if (copier) {
        delete copier;
        copier = nullptr;
    }
}

bool AudioManager::begin() {
    if (isInitialized) {
        return true;
    }

    // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Debug);
    // AudioDriverLogger.begin(Serial, AudioDriverLogLevel::Debug);

    ESP_LOGI(TAG, "Initializing audio pins...");
    DriverPins pins;
    pins.addI2C(PinFunction::CODEC, I2C_SCL, I2C_SDA);         // SCL=18, SDA=8
    pins.addI2S(PinFunction::CODEC, I2S_MCLK, I2S_SCLK, I2S_WS, I2S_DOUT); // MCLK=2, BCLK=17, WS=45, DOUT=15
    pins.addPin(PinFunction::PA, PA_PIN, PinLogic::Output);
    board.setPins(pins);

    // Start audio output
    ESP_LOGI(TAG, "Starting I2S & codec...");
    out.begin();
    // out.setAudioInfo(info);
    out.setVolume(0.7f);

    // Create audio task
    if (xTaskCreatePinnedToCore(audioTask, "audio", 4096, this, 10, &audioTaskHandle, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio task");
        return false;
    }

    isInitialized = true;
    return true;
}

void AudioManager::end() {
    if (!isInitialized) {
        return;
    }

    // Stop any current playback
    stop();

    // Clean up task
    if (audioTaskHandle != nullptr) {
        vTaskDelete(audioTaskHandle);
        audioTaskHandle = nullptr;
    }

    isInitialized = false;
    ESP_LOGI(TAG, "Audio system ended");
}

bool AudioManager::play(const char *urlStr) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "AudioManager not initialized. Call begin() first.");
        return false;
    }

    // Stop any currently playing audio
    stop();

    delay(10);
    url = new URLStream(); // completely reset the url.

    currentUrl = String(urlStr);

    ESP_LOGI(TAG, "Opening URL stream...");
    url->setTimeout(3000);
    ESP_LOGD(TAG, "URL begin start");
    if (!url->begin(urlStr, "audio/mpeg")) {
        ESP_LOGE(TAG, "URL open failed");
        return false;
    }
    ESP_LOGD(TAG, "URL begin ok");

    // Try to strip ID3 header only if enough data is available to avoid blocking
    fixRemoveID3Info(*url);

    mp3.begin();
    // Start the decoded stream
    decoded.begin(info);

    // Start the copier
    copier = new StreamCopy(decoded, *url, 4096);
    copier->begin();
    copier->setCheckAvailableForWrite(true);

    isPlaying = true;
    
    ESP_LOGI(TAG, "Audio playback started - isPlaying=%d", isPlaying);
    return true;
}

bool AudioManager::stop() {
    if (!isInitialized) {
        return false;
    }

    if (isPlaying) {
        isPlaying = false;
        currentUrl = "";

        // Clean up playback streams (not system resources)
        url->end(); // Close HTTP connection
        delete url;
        url = nullptr;
        copier->end(); // Stop stream copying and free buffers
        delete copier;
        copier = nullptr;
        decoded.end();
        mp3.end();

        // Reset audio configuration
        // out.setAudioInfo(info);

        ESP_LOGI(TAG, "Audio playback stopped");
        return true;
    }

    return false;
}

void AudioManager::setVolume(float volume) {
    if (isInitialized) {
        out.setVolume(volume);
    }
}

void AudioManager::fixRemoveID3Info(Stream &s) {
    // Avoid blocking: only proceed if at least 10 bytes are available
    if (s.available() < 10) {
        ESP_LOGD(TAG, "ID3 check skipped (available=%d)", s.available());
        return;
    }
    uint8_t hdr[10];
    if (s.readBytes(hdr, 10) != 10)
        return; // not enough -> ignore
    if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3') {
        uint32_t sz = ((hdr[6] & 0x7F) << 21) | ((hdr[7] & 0x7F) << 14) | ((hdr[8] & 0x7F) << 7) | (hdr[9] & 0x7F);
        uint32_t toSkip = sz; // header already consumed
        uint8_t buf[256];
        while (toSkip) {
            size_t n = s.readBytes(buf, toSkip > sizeof(buf) ? sizeof(buf) : toSkip);
            if (!n)
                break;
            toSkip -= n;
        }
    } else {
        // Not ID3v2: we consumed 10 bytes; simplest workaround is to re-open the URL
        // so we don't lose them:
        url->end();
        url->begin(currentUrl.c_str(), "audio/mpeg");
    }
}

void AudioManager::audioTask(void *parameter) {
    AudioManager *driver = static_cast<AudioManager *>(parameter);
    if (driver) {
        driver->processAudio();
    }
    vTaskDelete(nullptr);
}

void AudioManager::processAudio() {
    for(;;) {
        // Check if we should be playing audio
        if (isPlaying && url && copier) {
            size_t bytesCopied = copier->copy();
            
            // Use a single static counter across both branches
            static uint32_t zeroBytesStart = 0;
            
            // If no bytes copied and URL has no data, check if we should stop
            if (bytesCopied == 0) {
                zeroBytesStart++;
                if (zeroBytesStart > playbackCompleteCountThreshold) {
                    // Check if URL is also empty
                    if (url && url->available() == 0) {
                        ESP_LOGI(TAG, "Playback completed - no data copied and URL empty");
                        stop();
                        zeroBytesStart = 0;
                        continue;
                    }
                }
            } else {
                // Reset the zero-bytes counter when we copy data
                zeroBytesStart = 0;
            }
            
            vTaskDelay(1);
        } else {
            // If not playing, wait a bit before checking again
            vTaskDelay(100);
        }
    }
}
