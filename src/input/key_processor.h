#pragma once
#include "core/driver_interface.h"
#include "core/event_system.h"
#include "core/events.h"
#include <functional>

/**
 * @brief Key processor that handles keyboard input and function keys
 * 
 * Replaces the old KeyProcessing class with a cleaner interface
 * that uses events for decoupled communication.
 */
class KeyProcessor : public DriverInterface {
public:
    KeyProcessor();
    ~KeyProcessor();
    
    // DriverInterface implementation
    bool initialize() override;
    void shutdown() override;
    void tick() override;
    bool isReady() const override;
    
    // Key processing
    void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers);
    
    // Event subscriptions
    void onKeyEvent(std::function<void(const KeyEvent&)> callback);
    void onFunctionKeyEvent(std::function<void(const FunctionKeyEvent&)> callback);
    
    // Legacy compatibility
    void setSubmitCallback(void (*callback)());
    void setKeyInCallback(void (*callback)(char key));
    
private:
    // Key event queue for safe LVGL operations
    volatile KeyEvent pendingKeyEvent_;
    
    // Ring buffer for function actions
    static const int kFunctionQueueSize = 8;
    volatile uint8_t funcHead_;
    volatile uint8_t funcTail_;
    FunctionKeyEvent funcQueue_[kFunctionQueueSize];
    
    // Legacy callbacks
    void (*submitCallback_)();
    void (*keyInCallback_)(char key);
    
    // Event buses
    EventBus<KeyEvent>* keyEventBus_;
    EventBus<FunctionKeyEvent>* functionKeyEventBus_;
    
    // Helpers
    void enqueueFunction(FunctionKeyEvent::Type action);
    bool dequeueFunction(FunctionKeyEvent& actionOut);
    void processQueuedKeys();
    void processQueuedFunctions();
    FunctionKeyEvent::Type convertKeyCodeToFunction(uint8_t keyCode);
};
