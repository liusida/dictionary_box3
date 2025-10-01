#include "audio_manager.h"
#include "core_misc/log.h"
#include "drivers_i2c/i2c_manager.h"
#include "network_control.h"
#include "ui_status.h"

namespace dict {

static const char *TAG = "AudioManager";

extern NetworkControl *g_network;
extern StatusOverlay *g_status;

AudioManager::AudioManager()
    : board(AudioDriverES8311, NoPins), out(board), info(32000, 2, 16), player(nullptr), decoder(), urlSource(nullptr), urlStream(),
      initialized_(false), isPlaying(false), volume_(0.7f) {
  // Initialize preferences for volume persistence
  if (!preferences.begin("audio_config", false)) {
    ESP_LOGE(TAG, "Failed to open audio preferences");
  } else {
    ESP_LOGI(TAG, "Audio preferences initialized successfully");
  }
}

AudioManager::~AudioManager() { shutdown(); }

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
  pins.addI2C(PinFunction::CODEC, SHARED_I2C_SCL, SHARED_I2C_SDA, I2CManager::instance().getWire().getBusNum(), I2CManager::instance().getFrequency(),
              I2CManager::instance().getWire(), false);
  pins.addI2S(PinFunction::CODEC, I2S_MCLK, I2S_SCLK, I2S_WS, I2S_DOUT); // MCLK=2, BCLK=17, WS=45, DOUT=15
  pins.addPin(PinFunction::PA, PA_PIN, PinLogic::Output);
  board.setPins(pins);

  // Start audio output
  ESP_LOGI(TAG, "Starting I2S & codec...");
  if (!out.begin()) {
    ESP_LOGE(TAG, "Failed to initialize audio output stream (I2S & codec)");
    return false;
  }

  // Load saved volume from preferences, default to 0.7 if not found
  float savedVolume = preferences.getFloat("volume", 0.7f);
  volume_ = savedVolume;
  out.setVolume(volume_);
  ESP_LOGI(TAG, "Volume loaded from preferences: %.2f", volume_);

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

  initialized_ = false;
  ESP_LOGI(TAG, "AudioManager shutdown complete");
}

void AudioManager::tick() {
  if (!initialized_ || !player) {
    return;
  }
  if (player && player->getStream()) {
    if (player->isActive()) {
      try {
        player->copy();
      } catch (...) {
        ESP_LOGE(TAG, "player->copy() failed, ignoring.");
      }
    } else { // timeout detected, clean up
      if (isPlaying) {
        ESP_LOGI(TAG, "Player timeout detected, stopping and cleaning up");
        stop();
      }
    }
  }
}

bool AudioManager::play(const char *url) {
  if (!initialized_) {
    ESP_LOGE(TAG, "AudioManager not initialized");
    return false;
  }

  ESP_LOGI(TAG, "Playing: %s", url);

  // Clean up any existing player and sources
  if (isPlaying) {
    stop();
  }

  decoder.begin();

  // Create appropriate source based on URL
  if (!isUrl(url)) {
    ESP_LOGE(TAG, "URL is not a valid URL");
    return false;
  }

  createUrlSource(url);
  if (!urlSource) {
    ESP_LOGE(TAG, "Failed to create URL source");
    return false;
  }
  // Create player with URL source
  player = new AudioPlayer(*urlSource, out, decoder);

  if (!player) {
    ESP_LOGE(TAG, "Failed to create AudioPlayer");
    return false;
  }

  // Set up metadata callback
  // player->setMetadataCallback(staticMetadataCallback);
  // this has some bug. playing apple explanation will cause:
  //  [ 43841][I][audio_manager.cpp:266] staticMetadataCallback(): [AudioManager] Metadata [Title]: ��0
  //  [ 43842][I][audio_manager.cpp:266] staticMetadataCallback(): [AudioManager] Metadata [Artist]: �+[x��٬��T�␌␂V�␟␗���.r��Օ�*D
  //     �y��842][I][audio_manager.cpp:266] staticMetadataCallback(): [AudioManager] Metadata [Album]: �y�����6T�␘��ԗ��␚�␡��YeS
  //  [ 43842][I][audio_manager.cpp:266] staticMetadataCallback(): [AudioManager] Metadata [Other]: Drum Solo

  // Start playback
  g_status->updateAudioStatus(AudioState::Working, "mp3");
  if (player->begin()) {
    isPlaying = true;

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
    if (!player) {
      ESP_LOGE(TAG, "Player is null");
      return false;
    }
    ESP_LOGI(TAG, "Stopping playback");

    isPlaying = false; // Stop the player before cleaning up
    player->stop();
    decoder.clearNotifyAudioChange();
    g_status->updateAudioStatus(AudioState::Ready);
    player->end();
    delay(10);
    decoder.end();
    cleanupSources();
    delete player;
    player = nullptr;

    ESP_LOGI(TAG, "Playback stopped");
  }

  return true;
}

bool AudioManager::isCurrentlyPlaying() const { return isPlaying && player && player->isActive(); }

bool AudioManager::isCurrentlyPaused() const {
  return false; // AudioPlayer doesn't have pause functionality in this implementation
}

void AudioManager::setVolume(float volume) {
  if (!initialized_) {
    return;
  }

  if (volume < 0.0f)
    volume = 0.0f;
  if (volume > 1.0f)
    volume = 1.0f;
  volume_ = volume;
  out.setVolume(volume_);

  // Save volume to preferences for persistence across reboots
  preferences.putFloat("volume", volume_);

  ESP_LOGI(TAG, "Volume set to: %.2f (saved to preferences)", volume_);
}

bool AudioManager::isUrl(const char *path) const { return strstr(path, "http://") == path || strstr(path, "https://") == path; }

void AudioManager::createUrlSource(const char *url) {
  ESP_LOGI(TAG, "Creating URL source for: %s", url);

  client.setInsecure();
  urlStream.setClient(client);

  // Create URL source with single URL
  urlSource = new AudioSourceDynamicURLNoAutoNext(urlStream, "audio/mp3");
  urlSource->addURL(url);
  urlSource->setTimeoutAutoNext(2000); // if no data for 2 sec, stop the player

  if (!urlSource) {
    ESP_LOGE(TAG, "Failed to create AudioSourceURL");
  }
}

void AudioManager::cleanupSources() {
  if (urlSource) {
    delete urlSource;
    urlSource = nullptr;
  }
}

void AudioManager::staticMetadataCallback(MetaDataType type, const char *str, int len) {
  ESP_LOGI(TAG, "Metadata [%s]: %.*s",
           type == MetaDataType::Title    ? "Title"
           : type == MetaDataType::Artist ? "Artist"
           : type == MetaDataType::Album  ? "Album"
                                          : "Other",
           len, str);
}

} // namespace dict