#include "audio_manager.h"
#include "../drivers_i2c/i2c_manager.h"
#include "../core_eventing/event_publisher.h"
#include "../core_log/log.h"

namespace dict {

static const char *TAG = "AudioManager";
// Route allocations > 256 bytes to PSRAM if available
static MemoryManager s_audioMem(256);

AudioManager::AudioManager()
    : board(AudioDriverES8311, NoPins), out(board), info(32000, 2, 16), url(nullptr), fileStream(nullptr), mp3(), decoded(out, mp3), metadataFilter(decoded), copier(nullptr),
      localFileSource(nullptr), localFilePlayer(nullptr), audioTaskHandle(nullptr), isInitialized(false), isPlaying(false), currentUrl(""), audioMutex(nullptr), playbackCompleteCountThreshold(10),
      expectedContentLength(0) {
    audioMutex = xSemaphoreCreateMutex();
}

AudioManager::~AudioManager() {
    shutdown();
    if (url) {
        delete url;
        url = nullptr;
    }
    if (fileStream) {
        delete fileStream;
        fileStream = nullptr;
    }
    if (copier) {
        delete copier;
        copier = nullptr;
    }
    if (localFilePlayer) {
        delete localFilePlayer;
        localFilePlayer = nullptr;
    }
    if (localFileSource) {
        delete localFileSource;
        localFileSource = nullptr;
    }
    if (audioMutex) {
        vSemaphoreDelete(audioMutex);
        audioMutex = nullptr;
    }
}

bool AudioManager::initialize() {
    ESP_LOGI(TAG, "=== AudioManager::initialize() called ===");
    if (isInitialized) {
        ESP_LOGI(TAG, "AudioManager already initialized, returning true");
        return true;
    }

    // copier->setDelayOnNoData(500);

    ESP_LOGI(TAG, "Initializing audio pins...");
    
    // Ensure I2C is initialized before use
    if (!I2CManager::instance().isReady()) {
        ESP_LOGI(TAG, "I2C not ready, initializing...");
        if (!I2CManager::instance().initialize()) {
            ESP_LOGE(TAG, "Failed to initialize I2C for audio codec");
            return false;
        }
    }
    
    DriverPins pins;
    
    ESP_LOGI(TAG, "Bus Number: %d", I2CManager::instance().getWire().getBusNum());
    // I2C is already initialized by I2CManager - disable AudioTools I2C initialization
    pins.addI2C(PinFunction::CODEC, SHARED_I2C_SCL, SHARED_I2C_SDA, I2CManager::instance().getWire().getBusNum(), I2CManager::instance().getFrequency(), I2CManager::instance().getWire(), false);
    pins.addI2S(PinFunction::CODEC, I2S_MCLK, I2S_SCLK, I2S_WS, I2S_DOUT); // MCLK=2, BCLK=17, WS=45, DOUT=15
    pins.addPin(PinFunction::PA, PA_PIN, PinLogic::Output);
    board.setPins(pins);

    // Start audio output
    ESP_LOGI(TAG, "Starting I2S & codec...");
    if (!out.begin()) {
        ESP_LOGE(TAG, "Failed to initialize audio output stream (I2S & codec)");
        return false;
    }
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

void AudioManager::shutdown() {
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

void AudioManager::tick() {
    // Audio processing is handled in the background task
    // This method is here for future expansion
}

bool AudioManager::isReady() const { return isInitialized; }

bool AudioManager::play(const char *urlStr) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "AudioManager not initialized. Call begin() first.");
        return false;
    }

    if (xSemaphoreTake(audioMutex, 1000)) { // Wait for audio task to finish
        // Stop any currently playing audio
        stop();

        delay(10);
        
        currentUrl = String(urlStr);
        
        // Check if it's a local file path (starts with "/" and doesn't contain "://")
        bool isLocalFile = (urlStr[0] == '/' && strstr(urlStr, "://") == nullptr);
        
        if (isLocalFile) {
            ESP_LOGI(TAG, "Opening local file with AudioPlayer: %s", urlStr);
            
            // Clean up any existing streams
            if (url) {
                delete url;
                url = nullptr;
            }
            if (fileStream) {
                delete fileStream;
                fileStream = nullptr;
            }
            if (copier) {
                delete copier;
                copier = nullptr;
            }
            if (localFilePlayer) {
                delete localFilePlayer;
                localFilePlayer = nullptr;
            }
            if (localFileSource) {
                delete localFileSource;
                localFileSource = nullptr;
            }
            // Assuming LittleFS is already initialized
            // Create custom AudioSource that uses already-mounted LittleFS
            localFileSource = new AudioSourceLittleFSMounted();
            localFileSource->begin();
            
            // Create AudioPlayer with the source, output, and decoder
            localFilePlayer = new AudioPlayer(*localFileSource, out, mp3);
            
            // Set the file path and start playing
            if (!localFilePlayer->setPath(urlStr)) {
                ESP_LOGE(TAG, "Failed to set file path in AudioPlayer: %s", urlStr);
                delete localFilePlayer;
                localFilePlayer = nullptr;
                delete localFileSource;
                localFileSource = nullptr;
                xSemaphoreGive(audioMutex);
                return false;
            }
            
            // Start playing
            localFilePlayer->play();
            ESP_LOGI(TAG, "Local file playing with AudioPlayer: %s", urlStr);
            
            // For local files with AudioPlayer, we're done - no need for StreamCopy setup
            isPlaying = true;
            
            // Publish audio event
            EventPublisher::instance().publish(
                AudioEvent(AudioEvent::PlaybackStarted, String(urlStr))
            );
            ESP_LOGI(TAG, "Audio playback started - isPlaying=%d", isPlaying);
            
            xSemaphoreGive(audioMutex);
            return true;
        } else {
            ESP_LOGI(TAG, "Opening URL stream: %s", urlStr);
            
            // Clean up any existing streams
            if (url) {
                delete url;
                url = nullptr;
            }
            if (fileStream) {
                delete fileStream;
                fileStream = nullptr;
            }
            
            // Create new URL stream
            url = new URLStream();
            url->setTimeout(10000);
            url->httpRequest().header().setProtocol("HTTP/1.0");
            url->setConnectionClose(true);
            ESP_LOGD(TAG, "URL begin start");
            if (!url->begin(urlStr, "audio/mpeg")) {
                ESP_LOGE(TAG, "URL open failed for: %s", urlStr);
                xSemaphoreGive(audioMutex);
                return false;
            }
            ESP_LOGD(TAG, "URL begin ok - checking connection status");
        }

        // Check if we can read from the stream
        Stream* sourceStream = isLocalFile ? static_cast<Stream*>(fileStream) : static_cast<Stream*>(url);
        if (sourceStream->available() == 0) {
            ESP_LOGW(TAG, "Stream has no data available immediately");
        } else {
            ESP_LOGI(TAG, "Stream has %d bytes available", sourceStream->available());
        }

        if (isLocalFile) {
            // For local files, get file size
            expectedContentLength = fileStream->size();
            ESP_LOGI(TAG, "Local file size: %d bytes", expectedContentLength);
        } else {
            // For URL streams, get content length from HTTP headers
            expectedContentLength = url->httpRequest().contentLength();
            if (expectedContentLength > 0) {
                ESP_LOGI(TAG, "Expected content length: %d bytes", expectedContentLength);
            } else {
                ESP_LOGW(TAG, "No Content-Length header found");
                expectedContentLength = 1024 * 1024; // Unknown size, default to 1MB
            }
        }

        mp3.begin();
        // Start the decoded stream
        decoded.begin(info);

        // Start the metadata filter
        metadataFilter.begin();

        // Start the copier - copy from source stream to metadata filter
        copier = new StreamCopy(metadataFilter, *sourceStream, 4096);
        copier->begin();
        copier->setCheckAvailableForWrite(true);

        isPlaying = true;

        // Publish audio event
        EventPublisher::instance().publish(
            AudioEvent(AudioEvent::PlaybackStarted, String(urlStr))
        );
        ESP_LOGI(TAG, "Audio playback started - isPlaying=%d", isPlaying);

        xSemaphoreGive(audioMutex);
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to acquire audio mutex - audio task may be busy");
        return false;
    }
}

bool AudioManager::stop() {
    if (!isInitialized) {
        return false;
    }

    if (isPlaying) {
        isPlaying = false;
        currentUrl = "";

        // Clean up playback streams (not system resources)
        if (url) {
            url->end(); // Close HTTP connection
            delete url;
            url = nullptr;
        }
        if (fileStream) {
            fileStream->close(); // Close file
            delete fileStream;
            fileStream = nullptr;
        }
        if (copier) {
            copier->end(); // Stop stream copying and free buffers
            delete copier;
            copier = nullptr;
        }
        if (localFilePlayer) {
            localFilePlayer->stop();
            delete localFilePlayer;
            localFilePlayer = nullptr;
        }
        if (localFileSource) {
            delete localFileSource;
            localFileSource = nullptr;
        }
        metadataFilter.end();
        decoded.end();
        mp3.end();

        // Reset audio configuration
        // out.setAudioInfo(info);

        // Publish audio event
        EventPublisher::instance().publish(
            AudioEvent(AudioEvent::PlaybackStopped, currentUrl)
        );

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


void AudioManager::audioTask(void *parameter) {
    AudioManager *driver = static_cast<AudioManager *>(parameter);
    if (driver) {
        driver->processAudio();
    }
    vTaskDelete(nullptr);
}

void AudioManager::processAudio() {
    static uint32_t downloadedBytes = 0;
    for (;;) {
        // Check if we should be playing audio
        if (isPlaying) {
            if (localFilePlayer && localFilePlayer->isActive()) {
                // Handle local file playback with AudioPlayer
                if (xSemaphoreTake(audioMutex, 0)) {
                    localFilePlayer->copy();
                    xSemaphoreGive(audioMutex);
                    
                    // Check if playback is complete
                    if (!localFilePlayer->isActive()) {
                        ESP_LOGI(TAG, "Local file playback completed");
                        stop();
                        continue;
                    }
                }
                vTaskDelay(1);
            } else if (url && copier) {
                // Handle URL stream playback with StreamCopy
                if (url->available() == 0 && !*url) {
                    ESP_LOGW(TAG, "URL stream disconnected - stopping playback");
                    stop();
                    continue;
                }
                bool shouldStop = false;
                if (xSemaphoreTake(audioMutex, 0)) {
                    size_t bytesCopied = copier->copy();
                    xSemaphoreGive(audioMutex);
                    downloadedBytes += bytesCopied;
                    if (downloadedBytes >= expectedContentLength) {
                        shouldStop = true;
                    }
                }

                if (shouldStop) {
                    ESP_LOGI(TAG, "Download completed, %d bytes, %d bytes expected. Let's stop here.", downloadedBytes, expectedContentLength);
                    stop();
                    continue;
                }
                vTaskDelay(1);
            } else {
                // No valid playback method, wait
                vTaskDelay(100);
            }
        } else {
            // If not playing, wait a bit before checking again
            downloadedBytes = 0;
            vTaskDelay(100);
        }
    }
}

} // namespace dict
