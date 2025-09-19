#pragma once

#include "ui/ui.h"
#include <Arduino.h>

/**
 * @brief Controller for the Keyboard Settings screen
 * 
 * Handles keyboard settings configuration and UI interactions
 */
class KeyboardSettingsController {
public:
    KeyboardSettingsController();
    ~KeyboardSettingsController();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterKeyboardSettingsState();
    void exitKeyboardSettingsState();
    
private:
    bool initialized_;
};
