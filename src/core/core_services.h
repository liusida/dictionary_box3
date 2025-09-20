#pragma once
#include <Arduino.h>
#include <memory>
#include "driver_interface.h"

// Forward declarations
class AudioManager;
class DisplayManager;
class KeyProcessor;

/**
 * @brief Core services manager for Tier 1 (essential) services
 * 
 * Manages display, audio, and input processing services that are
 * essential for basic app functionality. These services must initialize
 * successfully for the app to work.
 */
class CoreServices {
public:
    static CoreServices& instance();
    
    // Service accessors
    AudioManager& audio();
    DisplayManager& display();
    KeyProcessor& keyProcessor();
    
    // Lifecycle management
    bool initialize();
    void shutdown();
    
    // Status checking
    bool isReady() const;
    
private:
    CoreServices() = default;
    ~CoreServices() = default;
    
    // Disable copy constructor and assignment operator
    CoreServices(const CoreServices&) = delete;
    CoreServices& operator=(const CoreServices&) = delete;
    
    // Service instances
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<DisplayManager> displayManager_;
    std::unique_ptr<KeyProcessor> keyProcessor_;
    
    // Initialization state
    bool initialized_ = false;
};
