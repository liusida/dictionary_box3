#include "event_publisher.h"
#include "core/log.h"

static const char *TAG = "EventPublisher";

EventPublisher& EventPublisher::instance() {
    static EventPublisher instance;
    return instance;
}

EventPublisher::EventPublisher() : eventSystem_(EventSystem::instance()) {
}

void EventPublisher::publishAppStateEvent(AppStateEvent::Type type) {
    AppStateEvent event(type);
    eventSystem_.getEventBus<AppStateEvent>().publish(event);
    ESP_LOGD(TAG, "Published AppStateEvent: %d", type);
}

void EventPublisher::publishAudioEvent(AudioEvent::Type type, const String& url, const String& error) {
    AudioEvent event(type, url, error);
    eventSystem_.getEventBus<AudioEvent>().publish(event);
    ESP_LOGD(TAG, "Published AudioEvent: %d, URL: %s", type, url.c_str());
}

void EventPublisher::publishWiFiEvent(WiFiEvent::Type type, const String& ssid, const IPAddress& ip, const String& error) {
    WiFiEvent event(type, ssid, ip, error);
    eventSystem_.getEventBus<WiFiEvent>().publish(event);
    ESP_LOGD(TAG, "Published WiFiEvent: %d, SSID: %s", type, ssid.c_str());
}

void EventPublisher::publishBLEEvent(BLEEvent::Type type, const String& address, const String& name, const String& error) {
    BLEEvent event(type, address, name, error);
    eventSystem_.getEventBus<BLEEvent>().publish(event);
    ESP_LOGD(TAG, "Published BLEEvent: %d, Address: %s", type, address.c_str());
}

void EventPublisher::publishDictionaryEvent(DictionaryEvent::Type type, const String& word, const String& explanation, const String& sample, const String& error) {
    DictionaryEvent event(type, word, explanation, sample, error);
    eventSystem_.getEventBus<DictionaryEvent>().publish(event);
    ESP_LOGD(TAG, "Published DictionaryEvent: %d, Word: %s", type, word.c_str());
}

void EventPublisher::publishUIEvent(UIEvent::Type type, const String& screen, const String& input, const String& button) {
    UIEvent event(type, screen, input, button);
    eventSystem_.getEventBus<UIEvent>().publish(event);
    ESP_LOGD(TAG, "Published UIEvent: %d, Screen: %s", type, screen.c_str());
}
