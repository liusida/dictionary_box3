#pragma once
#include "common.h"
#include "core_eventing/event_system.h"
#include "core_eventing/events.h"
#include <functional>

namespace dict {

class KeyProcessor {
public:
    // Constructor/Destructor
    KeyProcessor();
    ~KeyProcessor();

    // Core lifecycle methods
    bool initialize(); // Initialize key processor and event system
    void shutdown(); // Clean shutdown of key processor
    void tick(); // Process events (event-driven architecture)
    bool isReady() const; // Check if key processor is ready

    // Main functionality methods
    void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers); // Send key event to LVGL and event system

private:
    // Private member variables
    EventBus<KeyEvent>* keyEventBus_;
    EventBus<FunctionKeyEvent>* functionKeyEventBus_;

    // Private methods
    FunctionKeyEvent::Type convertKeyCodeToFunction(uint8_t keyCode);
};

} // namespace dict


