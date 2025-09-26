#pragma once
#include "dictionary.h"
#include "lvgl.h"

namespace dict {

class StatusOverlay {
public:
    // Constructor/Destructor
    StatusOverlay();
    ~StatusOverlay();

    // Core lifecycle methods
    bool initialize(); // Initialize status overlay and create UI elements
    void shutdown(); // Clean shutdown and free resources
    void tick(); // Update status indicators and handle animations
    bool isReady() const; // Check if status overlay is ready for use

    // Main functionality methods
    void attachToScreen(lv_obj_t* screen); // Attach overlay to any LVGL screen
    void detachFromScreen(); // Remove overlay from current screen
    void show(); // Show status indicators
    void hide(); // Hide status indicators
    bool isVisible() const; // Check if overlay is currently visible

    // Status update methods
    void updateWiFiStatus(bool connected, const String& ssid = ""); // Update WiFi connection status
    void updateBLEStatus(bool connected, const String& device = ""); // Update BLE keyboard status
    void updateAudioStatus(bool playing, const String& track = ""); // Update audio playback status

    // Configuration methods
    void setPosition(lv_align_t align, int32_t x, int32_t y); // Set overlay position on screen
    void setStyle(const lv_style_t* style); // Set custom style for indicators
    void setIndicatorSize(uint8_t size); // Set size of status indicators
    void setAnimationDuration(uint16_t duration); // Set animation duration for status changes
    
    // Blinking control methods
    void setWiFiBlinking(bool enable); // Enable/disable WiFi indicator blinking
    void setBLEBlinking(bool enable); // Enable/disable BLE indicator blinking
    void setAudioBlinking(bool enable); // Enable/disable Audio indicator blinking
    void setBlinkInterval(uint16_t intervalMs); // Set blinking interval in milliseconds

    // Utility/getter methods
    lv_obj_t* getContainer() const { return container_; } // Get LVGL container object
    bool isAttached() const; // Check if overlay is attached to a screen

private:
    // Private member variables
    lv_obj_t* null_screen_;
    lv_obj_t* container_;
    lv_obj_t* wifiIndicator_;
    lv_obj_t* bleIndicator_;
    lv_obj_t* audioIndicator_;
    lv_obj_t* attachedScreen_;
    
    bool initialized_;
    bool visible_;
    uint8_t indicatorSize_;
    uint16_t animationDuration_;
    
    // Status states
    bool wifiConnected_;
    bool bleConnected_;
    bool audioPlaying_;
    String wifiSSID_;
    String bleDevice_;
    String audioTrack_;
    
    // Indicator colors
    lv_color_t wifiColor_;
    lv_color_t bleColor_;
    lv_color_t audioColor_;
    
    // Blinking state
    bool wifiBlinking_;
    bool bleBlinking_;
    bool audioBlinking_;
    uint16_t blinkInterval_;
    unsigned long lastBlinkTime_;
    bool blinkState_; // Current blink state (true = visible, false = hidden)

    // Private methods
    void createIndicators(); // Create LVGL indicator objects
    void updateWiFiIndicator(); // Update WiFi indicator appearance
    void updateBLEIndicator(); // Update BLE indicator appearance
    void updateAudioIndicator(); // Update audio indicator appearance
    void applyIndicatorStyle(lv_obj_t* indicator, bool active, lv_color_t activeColor, bool blinking = false); // Apply style to indicator
    static void indicatorClickCallback(lv_event_t* e); // Handle indicator click events
};

} // namespace dict
