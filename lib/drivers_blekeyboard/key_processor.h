#pragma once
#include "event_system.h"
#include "events.h"
#include <functional>

using namespace core::eventing;

class KeyProcessor {
public:
    KeyProcessor();
    ~KeyProcessor();
    bool initialize();
    void shutdown();
    void tick();
    bool isReady() const;
    void sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers);
    void onKeyEvent(std::function<void(const KeyEvent&)> callback);
    void onFunctionKeyEvent(std::function<void(const FunctionKeyEvent&)> callback);
    void setSubmitCallback(void (*callback)());
    void setKeyInCallback(void (*callback)(char key));

private:
    volatile KeyEvent pendingKeyEvent_;
    static const int kFunctionQueueSize = 8;
    volatile uint8_t funcHead_;
    volatile uint8_t funcTail_;
    FunctionKeyEvent funcQueue_[kFunctionQueueSize];
    void (*submitCallback_)();
    void (*keyInCallback_)(char key);
    EventBus<KeyEvent>* keyEventBus_;
    EventBus<FunctionKeyEvent>* functionKeyEventBus_;
    void enqueueFunction(FunctionKeyEvent::Type action);
    bool dequeueFunction(FunctionKeyEvent& actionOut);
    void processQueuedKeys();
    void processQueuedFunctions();
    FunctionKeyEvent::Type convertKeyCodeToFunction(uint8_t keyCode);
};


