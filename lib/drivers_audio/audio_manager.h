#pragma once
#include "LittleFS.h"
#include "Preferences.h"
#include "audio_source_dynamic_url_no_auto_next.h"
#include "common.h"
#include "core_eventing/events.h"
#include <WiFi.h>
#define HELIX_LOG_LEVEL LogLevelHelix::Warning
#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "AudioTools/CoreAudio/AudioHttp/URLStream.h"
#include "AudioTools/CoreAudio/AudioPlayer.h"
#include "AudioTools/Disk/AudioSourceURL.h"

namespace dict {

class AudioManager {
public:
  // Constructor/Destructor
  AudioManager();
  ~AudioManager();

  // Core lifecycle methods
  bool initialize();                            // Initialize audio system and ES8311 codec
  void shutdown();                              // Clean shutdown of audio system and free resources
  void tick();                                  // Process audio events and state updates
  bool isReady() const { return initialized_; } // Check if audio system is ready for playback

  // Audio playback methods
  bool play(const char *url); // Play audio from URL
  bool stop();                // Stop current audio playback

  // Utility/getter methods
  bool isCurrentlyPlaying() const;            // Check if audio is currently playing
  bool isCurrentlyPaused() const;             // Check if audio is paused
  float getVolume() const { return volume_; } // Get current audio volume
  void setVolume(float volume);               // Set audio volume (0.0 to 1.0)

private:
  // Audio board and output (ES8311)
  AudioBoard board;
  AudioBoardStream out;
  AudioInfo info;

  // High-level player and decoder
  AudioPlayer *player;
  MP3DecoderHelix decoder;

  // Audio sources (created dynamically based on URL/file)
  WiFiClientSecure client;
  URLStream urlStream;
  AudioSourceDynamicURLNoAutoNext *urlSource;

  // State management
  bool initialized_;
  bool isPlaying;
  float volume_;
  Preferences preferences;

  // Private methods
  bool isUrl(const char *path) const;                                              // Check if path is a URL
  void createUrlSource(const char *url);                                           // Create URL source for playback
  void cleanupSources();                                                           // Clean up current sources
  static void staticMetadataCallback(MetaDataType type, const char *str, int len); // Static metadata callback
};

} // namespace dict