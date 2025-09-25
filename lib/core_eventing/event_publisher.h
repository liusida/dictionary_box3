#pragma once
#include "event_system.h"
#include "log.h"

namespace dict {

class EventPublisher {
public:
    static EventPublisher& instance();

    template<typename T>
    void publish(const T& event) {
        eventSystem_.getEventBus<T>().publish(event);
        ESP_LOGD(TAG, "Published event");
    }

private:
    EventPublisher();
    EventSystem& eventSystem_;
    static constexpr const char* TAG = "EventPublisher";
};

} // namespace dict
