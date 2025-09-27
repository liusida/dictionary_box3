#include "audio_manager.h"
#include "core_eventing/event_publisher.h"
#include "core_misc/log.h"
#include "drivers_i2c/i2c_manager.h"
#include "network_control.h"

namespace dict {

static const char *TAG = "AudioManager";

extern NetworkControl* g_network;

AudioManager::AudioManager()
    : board(AudioDriverES8311, NoPins), out(board), info(32000, 2, 16), 
      player(nullptr), decoder(), urlSource(nullptr), fileSource(nullptr), 
      urlStream(), initialized_(false), isPlaying(false), currentUrl(""), volume_(0.7f) {
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    ESP_LOGI(TAG, "=== AudioManager::initialize() called ===");
    if (initialized_) {
        ESP_LOGI(TAG, "AudioManager already initialized, returning true");
        return true;
    }

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
    out.setVolume(0.7f);

    // Don't create player here - create it when we have a source
    // This avoids initialization issues

    initialized_ = true;
    ESP_LOGI(TAG, "AudioManager initialized successfully");
    return true;
}

void AudioManager::shutdown() {
    if (!initialized_) {
        return;
    }

    ESP_LOGI(TAG, "Shutting down AudioManager...");
    
    // Stop any current playback
    if (isPlaying) {
        stop();
    }

    // Clean up player
    if (player) {
        player->end();
        delete player;
        player = nullptr;
    }

    // Clean up sources
    cleanupSources();

    // Don't call out.end() as it conflicts with shared I2C resources
    // The AudioBoard will be cleaned up when the object is destroyed
    // out.end();

    initialized_ = false;
    ESP_LOGI(TAG, "AudioManager shutdown complete");
}

void AudioManager::tick() {
    if (!initialized_ || !player) {
        return;
    }
    if (player->getStream() && player->getStream()->available()) {
        player->copy();
    }
}

bool AudioManager::play(const char* url) {
    if (!initialized_) {
        ESP_LOGE(TAG, "AudioManager not initialized");
        return false;
    }

    ESP_LOGI(TAG, "Playing: %s", url);
    
    // Clean up any existing player and sources
    if (player) {
        player->end();
        delete player;
        player = nullptr;
    }
    cleanupSources();
    
    // Create appropriate source based on URL/file
    if (isUrl(url)) {
        createUrlSource(url);
        if (!urlSource) {
            ESP_LOGE(TAG, "Failed to create URL source");
            return false;
        }
        // Create player with URL source
        player = new AudioPlayer(*urlSource, out, decoder);
    } else {
        createFileSource(url);
        if (!fileSource) {
            ESP_LOGE(TAG, "Failed to create file source");
            return false;
        }
        // Create player with file source
        player = new AudioPlayer(*fileSource, out, decoder);
    }

    if (!player) {
        ESP_LOGE(TAG, "Failed to create AudioPlayer");
        return false;
    }

    // Set up metadata callback
    player->setMetadataCallback(staticMetadataCallback);

    // Start playback
    if (player->begin()) {
        currentUrl = String(url);
        isPlaying = true;
        
        // Publish audio event
        EventPublisher::instance().publish(AudioEvent(AudioEvent::PlaybackStarted, String(url)));
        
        ESP_LOGI(TAG, "Playback started successfully");
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to start playback");
        return false;
    }
}

bool AudioManager::stop() {
    if (!initialized_ || !player) {
        return false;
    }

    if (isPlaying) {
        ESP_LOGI(TAG, "Stopping playback");
        
        player->stop();
        isPlaying = false;
        
        // Publish audio event
        EventPublisher::instance().publish(AudioEvent(AudioEvent::PlaybackStopped, currentUrl));
        
        currentUrl = "";
        ESP_LOGI(TAG, "Playback stopped");
    }
    
    return true;
}

bool AudioManager::isCurrentlyPlaying() const {
    return isPlaying && player && player->isActive();
}

bool AudioManager::isCurrentlyPaused() const {
    return false; // AudioPlayer doesn't have pause functionality in this implementation
}

void AudioManager::setVolume(float volume) {
    if (!initialized_ || !player) {
        return;
    }
    
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    volume_ = volume;
    out.setVolume(volume_);
    ESP_LOGI(TAG, "Volume set to: %.2f", volume_);
}

bool AudioManager::isUrl(const char* path) const {
    return strstr(path, "http://") == path || strstr(path, "https://") == path;
}

void AudioManager::createUrlSource(const char* url) {
    ESP_LOGI(TAG, "Creating URL source for: %s", url);

    WiFiClientSecure client;
    client.setInsecure();
    urlStream.setClient(client);

    // Create URL source with single URL
    urlSource = new AudioSourceDynamicURLNoAutoNext(urlStream, "audio/mp3");
    urlSource->addURL(url);

    if (!urlSource) {
        ESP_LOGE(TAG, "Failed to create AudioSourceURL");
    }
}

void AudioManager::createFileSource(const char* filePath) {
    ESP_LOGI(TAG, "Creating file source for: %s", filePath);
    
    // Create LittleFS source
    fileSource = new AudioSourceLittleFSMounted();
    
    if (!fileSource) {
        ESP_LOGE(TAG, "Failed to create AudioSourceLittleFSMounted");
    }
}

void AudioManager::cleanupSources() {
    if (urlSource) {
        delete urlSource;
        urlSource = nullptr;
    }
    
    if (fileSource) {
        delete fileSource;
        fileSource = nullptr;
    }
}

void AudioManager::staticMetadataCallback(MetaDataType type, const char* str, int len) {
    ESP_LOGI(TAG, "Metadata [%s]: %.*s", 
             type == MetaDataType::Title ? "Title" :
             type == MetaDataType::Artist ? "Artist" :
             type == MetaDataType::Album ? "Album" : "Other",
             len, str);
}

} // namespace dict