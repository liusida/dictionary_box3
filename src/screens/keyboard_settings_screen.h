#pragma once
#include "ui/ui.h"

// Forward declarations for drivers
class BLEKeyboard;

/**
 * @brief Simplified keyboard settings screen manager
 * 
 * Handles keyboard settings screen logic and user interactions.
 * Simplified version of the original keyboard settings controller.
 */
class KeyboardSettingsScreen {
public:
    KeyboardSettingsScreen(BLEKeyboard& bleDriver);
    ~KeyboardSettingsScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void showKeyboardSettingsScreen();
    
private:
    void setupUI();
    void startBLEScan();
    void onScanButtonPressed();
    void onConnectButtonPressed();
    void updateUI();
    
    // Driver (following simplified architecture)
    BLEKeyboard& bleDriver_;
    
    bool initialized_;
    
    /**
     * @brief Update status icons based on driver state (following simplified architecture)
     */
    void updateStatusIcons();
};
