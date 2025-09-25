#pragma once
#include "event_system.h"
#include "events.h"
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
    void tick(); // Process queued keys and function events
    bool isReady() const; // Check if key processor is ready

    // Main functionality methods
    void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers); // Send key event to LVGL and event system
    void onKeyEvent(std::function<void(const KeyEvent&)> callback); // Set callback for key events
    void onFunctionKeyEvent(std::function<void(const FunctionKeyEvent&)> callback); // Set callback for function key events
    void setSubmitCallback(void (*callback)()); // Set callback for submit action
    void setKeyInCallback(void (*callback)(char key)); // Set callback for key input

private:
    // Private member variables
    volatile KeyEvent pendingKeyEvent_;
    static const int kFunctionQueueSize = 8;
    volatile uint8_t funcHead_;
    volatile uint8_t funcTail_;
    FunctionKeyEvent funcQueue_[kFunctionQueueSize];
    void (*submitCallback_)();
    void (*keyInCallback_)(char key);
    EventBus<KeyEvent>* keyEventBus_;
    EventBus<FunctionKeyEvent>* functionKeyEventBus_;

    // Private methods
    void enqueueFunction(FunctionKeyEvent::Type action);
    bool dequeueFunction(FunctionKeyEvent& actionOut);
    void processQueuedKeys();
    void processQueuedFunctions();
    FunctionKeyEvent::Type convertKeyCodeToFunction(uint8_t keyCode);
};

} // namespace dict


