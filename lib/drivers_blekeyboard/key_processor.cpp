#include "key_processor.h"
#include "core_misc/log.h"
#include "core_misc/utils.h"
#include "lvgl.h"

namespace dict {

static const char *TAG = "KeyProcessor";

KeyProcessor::KeyProcessor() {
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
}

bool KeyProcessor::isReady() const { return true; }

void KeyProcessor::sendKeyToLVGL(char key, uint8_t key1, uint8_t modifiers) {
    if (key != 0) {
        ESP_LOGD(TAG, "Key: '%c' (0x%02X)", key, (unsigned char)key);
        KeyEvent event{key, key1, modifiers, true};
        keyEventBus_->publish(event);
    } else {
        ESP_LOGD(TAG, "Functional key: %d, %d", key1, modifiers);
        FunctionKeyEvent::Type action = convertKeyCodeToFunction(key1);
        if (action != FunctionKeyEvent::None) {
            FunctionKeyEvent event{action};
            functionKeyEventBus_->publish(event);
        }
    }
}

FunctionKeyEvent::Type KeyProcessor::convertKeyCodeToFunction(uint8_t keyCode) {
    switch (keyCode) {
        // HID usage IDs: F1=58, F2=59, F3=60, F4=61, F5=62, F6=63, F7=64, F8=65, F9=66, F10=67, F11=68, F12=69
        case 67: return FunctionKeyEvent::VolumeDown;       // F10 -> VolumeDown
        case 68: return FunctionKeyEvent::VolumeUp;         // F11 -> VolumeUp
        case 69: return FunctionKeyEvent::WifiSettings;     // F12 -> WifiSettings
        case 58: return FunctionKeyEvent::PrintMemoryStatus; // F1  -> PrintMemoryStatus
        case 59: return FunctionKeyEvent::ReadWord;          // F2  -> ReadWord
        case 60: return FunctionKeyEvent::ReadExplanation;   // F3  -> ReadExplanation
        case 61: return FunctionKeyEvent::ReadSampleSentence; // F4  -> ReadSampleSentence
        case 81: return FunctionKeyEvent::DownArrow;            // Down Arrow -> DownArrow
        case 82: return FunctionKeyEvent::UpArrow;            // Up Arrow -> UpArrow
        case 80: return FunctionKeyEvent::LeftArrow;            // Left Arrow -> LeftArrow
        case 79: return FunctionKeyEvent::RightArrow;            // Right Arrow -> RightArrow
        default: return FunctionKeyEvent::None;
    }
}

} // namespace dict


