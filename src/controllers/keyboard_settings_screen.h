#pragma once

#include "ui/ui.h"
#include <Arduino.h>
#include <vector>

/**
 * @brief Keyboard Settings screen manager
 * 
 * Handles keyboard settings configuration and UI interactions
 */
class KeyboardSettingsScreen {
public:
    KeyboardSettingsScreen();
    ~KeyboardSettingsScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterKeyboardSettingsState();
    void exitKeyboardSettingsState();
    
private:
    bool initialized_;
    static KeyboardSettingsScreen* instance_;
    
    void loadScreen(lv_obj_t* screen);
    void addObjectToDefaultGroup(lv_obj_t* obj);
    void scanAndPopulateDevices();
    static void scanButtonCallback(lv_event_t * e);
    static void connectButtonCallback(lv_event_t * e);
};
