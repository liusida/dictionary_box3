#pragma once
#include "common.h"
#include "lvgl.h"

namespace dict {

enum class WiFiState { None, Ready, Working };

enum class AudioState { None, Ready, Working };

class StatusOverlay {
public:
  // Singleton access
  static StatusOverlay &instance(); // Get singleton instance

  // Core lifecycle methods
  bool initialize();    // Initialize status overlay and create UI elements
  void shutdown();      // Clean shutdown and free resources
  bool isReady() const; // Check if status overlay is ready for use

  // Main functionality methods
  void attachToScreen(lv_obj_t *screen); // Attach overlay to any LVGL screen
  void detachFromScreen();               // Remove overlay from current screen
  void show();                           // Show status indicators
  void hide();                           // Hide status indicators
  bool isVisible() const;                // Check if overlay is currently visible

  // Status update methods
  void updateWiFiStatus(WiFiState state, const String &ssid = "");    // Update WiFi connection status
  void updateBLEStatus(bool connected, const String &device = "");    // Update BLE keyboard status
  void updateAudioStatus(AudioState state, const String &track = ""); // Update audio playback status

  // Configuration methods
  void setPosition(lv_align_t align, int32_t x, int32_t y); // Set overlay position on screen
  void setStyle(const lv_style_t *style);                   // Set custom style for indicators
  void setIndicatorSize(uint8_t size);                      // Set size of status indicators
  void setAnimationDuration(uint16_t duration);             // Set animation duration for status changes

  // Utility/getter methods
  lv_obj_t *getContainer() const { return container_; } // Get LVGL container object
  bool isAttached() const;                              // Check if overlay is attached to a screen

private:
  StatusOverlay();
  ~StatusOverlay() = default;
  StatusOverlay(const StatusOverlay &) = delete;
  StatusOverlay &operator=(const StatusOverlay &) = delete;

  // Private member variables
  lv_obj_t *null_screen_;
  lv_obj_t *container_;
  lv_obj_t *wifiIndicator_;
  lv_obj_t *bleIndicator_;
  lv_obj_t *audioIndicator_;
  lv_obj_t *attachedScreen_;

  bool initialized_;
  bool visible_;
  uint8_t indicatorSize_;
  uint16_t animationDuration_;

  // Status states
  WiFiState wifiState_;
  bool bleConnected_;
  AudioState audioState_;

  // Indicator colors
  lv_color_t wifiColor_;
  lv_color_t bleColor_;
  lv_color_t audioColor_;

  // Private methods
  void createIndicators();                                                            // Create LVGL indicator objects
  void updateWiFiIndicator();                                                         // Update WiFi indicator appearance
  void updateBLEIndicator();                                                          // Update BLE indicator appearance
  void updateAudioIndicator();                                                        // Update audio indicator appearance
  void applyIndicatorStyle(lv_obj_t *indicator, bool active, lv_color_t activeColor); // Apply style to indicator
};

} // namespace dict
