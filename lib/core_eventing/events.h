#pragma once
#include "common.h"

namespace dict {

// Key events
struct KeyEvent {
    char key;
    uint8_t keyCode;
    uint8_t modifiers;
    bool valid;
};

// Function Key Event (for internal KeyProcessor use)
struct FunctionKeyEvent {
    enum Type {
        None = 0,
        VolumeDown,
        VolumeUp,
        WifiSettings,
        PrintMemoryStatus,
        ReadWord,
        ReadExplanation,
        ReadSampleSentence,
        DownArrow,
        UpArrow,
        LeftArrow,
        RightArrow,
        Escape
    };
    Type type;
    
    FunctionKeyEvent() : type(None) {}
    FunctionKeyEvent(Type t) : type(t) {}
    FunctionKeyEvent(const FunctionKeyEvent& other) : type(other.type) {}
    FunctionKeyEvent& operator=(const FunctionKeyEvent& other) {
        type = other.type;
        return *this;
    }
    FunctionKeyEvent& operator=(FunctionKeyEvent&& other) {
        type = other.type;
        return *this;
    }
};

} // namespace dict
