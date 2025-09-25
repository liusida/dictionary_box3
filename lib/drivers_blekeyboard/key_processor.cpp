#include "key_processor.h"
#include "log.h"
#include "lvgl.h"
#include "utils.h"

using namespace core::eventing;

static const char *TAG = "KeyProcessor";

KeyProcessor::KeyProcessor() 
    : pendingKeyEvent_{0, 0, 0, false}, funcHead_(0), funcTail_(0), 
      submitCallback_(nullptr), keyInCallback_(nullptr) {
    for (int i = 0; i < kFunctionQueueSize; ++i) {
        funcQueue_[i] = {FunctionKeyEvent::None};
    }
    keyEventBus_ = &EventSystem::instance().getEventBus<KeyEvent>();
    functionKeyEventBus_ = &EventSystem::instance().getEventBus<FunctionKeyEvent>();
}

KeyProcessor::~KeyProcessor() {
    shutdown();
}

bool KeyProcessor::initialize() {
    ESP_LOGI(TAG, "Initializing key processor...");
    return true;
}

void KeyProcessor::shutdown() {
    ESP_LOGI(TAG, "Shutting down key processor...");
}

void KeyProcessor::tick() {
    processQueuedKeys();
    processQueuedFunctions();
}

bool KeyProcessor::isReady() const { return true; }

void KeyProcessor::sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) {
    if (key != 0) {
        pendingKeyEvent_.key = key;
        pendingKeyEvent_.keyCode = key1;
        pendingKeyEvent_.modifiers = modifiers;
        pendingKeyEvent_.valid = true;
        ESP_LOGD(TAG, "Queued key: '%c' (0x%02X)", key, (unsigned char)key);
        KeyEvent event{key, key1, modifiers, true};
        keyEventBus_->publish(event);
    } else {
        ESP_LOGD(TAG, "Functional key: %d, %d", key1, modifiers);
        FunctionKeyEvent::Type action = convertKeyCodeToFunction(key1);
        if (action != FunctionKeyEvent::None) {
            enqueueFunction(action);
            FunctionKeyEvent event{action};
            functionKeyEventBus_->publish(event);
        }
    }
}

void KeyProcessor::onKeyEvent(std::function<void(const KeyEvent&)> callback) { keyEventBus_->subscribe(callback); }
void KeyProcessor::onFunctionKeyEvent(std::function<void(const FunctionKeyEvent&)> callback) { functionKeyEventBus_->subscribe(callback); }
void KeyProcessor::setSubmitCallback(void (*callback)()) { submitCallback_ = callback; }
void KeyProcessor::setKeyInCallback(void (*callback)(char key)) { keyInCallback_ = callback; }

void KeyProcessor::enqueueFunction(FunctionKeyEvent::Type action) {
    uint8_t nextHead = (funcHead_ + 1) % kFunctionQueueSize;
    if (nextHead == funcTail_) { return; }
    funcQueue_[funcHead_] = {action};
    funcHead_ = nextHead;
}

bool KeyProcessor::dequeueFunction(FunctionKeyEvent& actionOut) {
    if (funcHead_ == funcTail_) { return false; }
    actionOut = funcQueue_[funcTail_];
    funcQueue_[funcTail_] = {FunctionKeyEvent::None};
    funcTail_ = (funcTail_ + 1) % kFunctionQueueSize;
    return true;
}

void KeyProcessor::processQueuedKeys() {
    if (!pendingKeyEvent_.valid) { return; }
    char key = pendingKeyEvent_.key;
    pendingKeyEvent_.valid = false;
    // LVGL handling moved to drivers_display/lvgl_helper via KeyEvent subscription
    (void)key;
}

void KeyProcessor::processQueuedFunctions() {
    static float currentVolume = 0.7f;
    FunctionKeyEvent action;
    while (dequeueFunction(action)) {
        switch (action.type) {
            case FunctionKeyEvent::VolumeDown:
                currentVolume -= 0.05f;
                if (currentVolume < 0.0f) currentVolume = 0.0f;
                ESP_LOGD(TAG, "Volume: %f", currentVolume);
                break;
            case FunctionKeyEvent::VolumeUp:
                currentVolume += 0.05f;
                if (currentVolume > 1.0f) currentVolume = 1.0f;
                ESP_LOGD(TAG, "Volume: %f", currentVolume);
                break;
            case FunctionKeyEvent::PrintMemoryStatus:
                printMemoryStatus();
                break;
            case FunctionKeyEvent::ReadWord:
            case FunctionKeyEvent::ReadExplanation:
            case FunctionKeyEvent::ReadSampleSentence:
                break;
            default:
                break;
        }
    }
}

FunctionKeyEvent::Type KeyProcessor::convertKeyCodeToFunction(uint8_t keyCode) {
    switch (keyCode) {
        // HID usage IDs: F1=58, F2=59, F3=60, F4=61, F5=62, F6=63, F7=64, F8=65, F9=66, F10=67, F11=68, F12=69
        case 67: return FunctionKeyEvent::VolumeDown;       // F10 -> VolumeDown
        case 68: return FunctionKeyEvent::VolumeUp;         // F11 -> VolumeUp
        case 58: return FunctionKeyEvent::PrintMemoryStatus; // F1  -> PrintMemoryStatus
        case 59: return FunctionKeyEvent::ReadWord;          // F2  -> ReadWord
        case 60: return FunctionKeyEvent::ReadExplanation;   // F3  -> ReadExplanation
        case 61: return FunctionKeyEvent::ReadSampleSentence; // F4  -> ReadSampleSentence
        default: return FunctionKeyEvent::None;
    }
}


