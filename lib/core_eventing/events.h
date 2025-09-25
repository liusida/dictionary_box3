#pragma once
#include <Arduino.h>

// Undefine Arduino's word macro to avoid conflicts
#ifdef word
#undef word
#endif

namespace dict {

// Key events
struct KeyEvent {
    char key;
    uint8_t keyCode;
    uint8_t modifiers;
    bool valid;
};

// Application state events
struct AppStateEvent {
    enum Type {
        EnteringSplash,
        EnteringMain,
        SystemReady,
        SystemNotReady
    };
    Type type;
    unsigned long timestamp;
    
    AppStateEvent(Type t) : type(t), timestamp(millis()) {}
};

// Audio events
struct AudioEvent {
    enum Type {
        PlaybackStarted,
        PlaybackStopped,
        PlaybackCompleted,
        PlaybackError
    };
    Type type;
    String url;
    String errorMessage;
    unsigned long timestamp;
    
    AudioEvent(Type t, const String& u = "", const String& err = "") 
        : type(t), url(u), errorMessage(err), timestamp(millis()) {}
};

// WiFi events
struct WiFiEvent {
    enum Type {
        Connected,
        Disconnected,
        ConnectionFailed,
        SettingsUIShown,
        SettingsUIClosed
    };
    Type type;
    String ssid;
    IPAddress ip;
    String errorMessage;
    unsigned long timestamp;
    
    WiFiEvent(Type t, const String& s = "", const IPAddress& i = IPAddress(), const String& err = "") 
        : type(t), ssid(s), ip(i), errorMessage(err), timestamp(millis()) {}
};

// BLE events
struct BLEEvent {
    enum Type {
        Connected,
        Disconnected,
        ScanStarted,
        ScanStopped,
        DeviceFound,
        ConnectionFailed
    };
    Type type;
    String deviceAddress;
    String deviceName;
    String errorMessage;
    unsigned long timestamp;
    
    BLEEvent(Type t, const String& addr = "", const String& name = "", const String& err = "") 
        : type(t), deviceAddress(addr), deviceName(name), errorMessage(err), timestamp(millis()) {}
};

// Dictionary events
struct DictionaryEvent {
    enum Type {
        LookupStarted,
        LookupCompleted,
        LookupFailed,
        AudioRequested
    };
    Type type;
    String word;
    String explanation;
    String sampleSentence;
    String errorMessage;
    unsigned long timestamp;
    
    DictionaryEvent(Type t, const String& w = "", const String& e = "", const String& s = "", const String& err = "") 
        : type(t), word(w), explanation(e), sampleSentence(s), errorMessage(err), timestamp(millis()) {}
};

// Function Key Event (for internal KeyProcessor use)
struct FunctionKeyEvent {
    enum Type {
        None = 0,
        VolumeDown,
        VolumeUp,
        PrintMemoryStatus,
        ReadWord,
        ReadExplanation,
        ReadSampleSentence
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

// UI events
struct UIEvent {
    enum Type {
        ScreenChanged,
        InputFocused,
        InputBlurred,
        ButtonPressed,
        FormSubmitted
    };
    Type type;
    String screenName;
    String inputText;
    String buttonName;
    unsigned long timestamp;
    
    UIEvent(Type t, const String& screen = "", const String& input = "", const String& button = "") 
        : type(t), screenName(screen), inputText(input), buttonName(button), timestamp(millis()) {}
};

} // namespace dict
