#pragma once
#include "event_system.h"
#include "events.h"

/**
 * @brief Helper class for publishing events
 * 
 * Provides convenient methods for publishing common events
 * throughout the system.
 */
class EventPublisher {
public:
    static EventPublisher& instance();
    
    // Application events
    void publishAppStateEvent(AppStateEvent::Type type);
    
    // Audio events
    void publishAudioEvent(AudioEvent::Type type, const String& url = "", const String& error = "");
    
    // WiFi events
    void publishWiFiEvent(WiFiEvent::Type type, const String& ssid = "", const IPAddress& ip = IPAddress(), const String& error = "");
    
    // BLE events
    void publishBLEEvent(BLEEvent::Type type, const String& address = "", const String& name = "", const String& error = "");
    
    // Dictionary events
    void publishDictionaryEvent(DictionaryEvent::Type type, const String& word = "", const String& explanation = "", const String& sample = "", const String& error = "");
    
    // UI events
    void publishUIEvent(UIEvent::Type type, const String& screen = "", const String& input = "", const String& button = "");
    
private:
    EventPublisher();
    EventSystem& eventSystem_;
};
