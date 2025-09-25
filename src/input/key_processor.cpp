#include "key_processor.h"
#include "audio_manager.h"
#include "utils.h"
#include "core/log.h"
#include "lvgl.h"

static const char *TAG = "KeyProcessor";

KeyProcessor::KeyProcessor() 
    : pendingKeyEvent_{0, 0, 0, false}, funcHead_(0), funcTail_(0), 
      submitCallback_(nullptr), keyInCallback_(nullptr) {
    
    for (int i = 0; i < kFunctionQueueSize; ++i) {
        funcQueue_[i] = {FunctionKeyEvent::None};
    }
    
    // Get event buses
    keyEventBus_ = &EventSystem::instance().getEventBus<KeyEvent>();
    functionKeyEventBus_ = &EventSystem::instance().getEventBus<FunctionKeyEvent>();
}

KeyProcessor::~KeyProcessor() {
    shutdown();
}

bool KeyProcessor::initialize() {
    ESP_LOGI(TAG, "Initializing key processor...");
    // Key processor doesn't need special initialization
    return true;
}

void KeyProcessor::shutdown() {
    ESP_LOGI(TAG, "Shutting down key processor...");
    // Clean up any resources if needed
}

void KeyProcessor::tick() {
    processQueuedKeys();
    processQueuedFunctions();
}

bool KeyProcessor::isReady() const {
    return true; // Key processor is always ready
}

void KeyProcessor::sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) {
    if (key != 0) {
        // Regular key
        pendingKeyEvent_.key = key;
        pendingKeyEvent_.keyCode = key1;
        pendingKeyEvent_.modifiers = modifiers;
        pendingKeyEvent_.valid = true;
        
        ESP_LOGD(TAG, "Queued key: '%c' (0x%02X)", key, (unsigned char)key);
        
        // Publish key event
        KeyEvent event{key, key1, modifiers, true};
        keyEventBus_->publish(event);
    } else {
        // Function key
        ESP_LOGD(TAG, "Functional key: %d, %d", key1, modifiers);
        FunctionKeyEvent::Type action = convertKeyCodeToFunction(key1);
        if (action != FunctionKeyEvent::None) {
            enqueueFunction(action);
            
            // Publish function key event
            FunctionKeyEvent event{action};
            functionKeyEventBus_->publish(event);
        }
    }
}

void KeyProcessor::onKeyEvent(std::function<void(const KeyEvent&)> callback) {
    keyEventBus_->subscribe(callback);
}

void KeyProcessor::onFunctionKeyEvent(std::function<void(const FunctionKeyEvent&)> callback) {
    functionKeyEventBus_->subscribe(callback);
}

void KeyProcessor::setSubmitCallback(void (*callback)()) {
    submitCallback_ = callback;
}

void KeyProcessor::setKeyInCallback(void (*callback)(char key)) {
    keyInCallback_ = callback;
}

void KeyProcessor::enqueueFunction(FunctionKeyEvent::Type action) {
    uint8_t nextHead = (funcHead_ + 1) % kFunctionQueueSize;
    if (nextHead == funcTail_) {
        return; // Queue full
    }
    funcQueue_[funcHead_] = {action};
    funcHead_ = nextHead;
}

bool KeyProcessor::dequeueFunction(FunctionKeyEvent& actionOut) {
    if (funcHead_ == funcTail_) {
        return false; // Queue empty
    }
    actionOut = funcQueue_[funcTail_];
    funcQueue_[funcTail_] = {FunctionKeyEvent::None};
    funcTail_ = (funcTail_ + 1) % kFunctionQueueSize;
    return true;
}

void KeyProcessor::processQueuedKeys() {
    if (!pendingKeyEvent_.valid) {
        return;
    }

    char key = pendingKeyEvent_.key;
    pendingKeyEvent_.valid = false;

    lv_group_t *group = lv_group_get_default();
    lv_obj_t *focused = lv_group_get_focused(group);

    ESP_LOGD(TAG, "Processing key: '%c' (0x%02X), Group=%p, Focused=%p", 
             key, (unsigned char)key, (void *)group, (void *)focused);

    if (focused) {
        if (lv_obj_has_class(focused, &lv_textarea_class)) {
            ESP_LOGD(TAG, "Processing text area input");
            if (key == 0x08) {
                lv_textarea_delete_char(focused);
                if (keyInCallback_) {
                    keyInCallback_(key);
                }
            } else if (key == '\n') {
                lv_textarea_t *ta = (lv_textarea_t *)focused;
                if (ta->one_line) {
                    if (submitCallback_) {
                        submitCallback_();
                    }
                } else {
                    lv_textarea_add_char(focused, '\n');
                }
            } else if (key >= 32 && key <= 126) {
                lv_textarea_add_char(focused, key);
                if (keyInCallback_) {
                    keyInCallback_(key);
                }
            }
        } else {
            ESP_LOGV(TAG, "Focused object is not a text area");
        }
    } else {
        ESP_LOGV(TAG, "No focused object");
    }
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
                // TODO: Volume control should be handled by screen controllers with audio driver access
                // Services::instance().audio().setVolume(currentVolume);
                break;
                
            case FunctionKeyEvent::VolumeUp:
                currentVolume += 0.05f;
                if (currentVolume > 1.0f) currentVolume = 1.0f;
                ESP_LOGD(TAG, "Volume: %f", currentVolume);
                // TODO: Volume control should be handled by screen controllers with audio driver access
                // Services::instance().audio().setVolume(currentVolume);
                break;
                
            case FunctionKeyEvent::PrintMemoryStatus:
                printMemoryStatus();
                break;
                
            case FunctionKeyEvent::ReadWord:
            case FunctionKeyEvent::ReadExplanation:
            case FunctionKeyEvent::ReadSampleSentence:
                // These will be handled by the main screen controller
                // through event subscriptions
                break;
                
            default:
                break;
        }
    }
}

FunctionKeyEvent::Type KeyProcessor::convertKeyCodeToFunction(uint8_t keyCode) {
    switch (keyCode) {
        case 67: return FunctionKeyEvent::VolumeDown;
        case 68: return FunctionKeyEvent::VolumeUp;
        case 58: return FunctionKeyEvent::PrintMemoryStatus;
        case 59: return FunctionKeyEvent::ReadWord;
        case 60: return FunctionKeyEvent::ReadExplanation;
        case 61: return FunctionKeyEvent::ReadSampleSentence;
        default: return FunctionKeyEvent::None;
    }
}
