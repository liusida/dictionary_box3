#pragma once
#include "common.h"
#include "event_system.h"

namespace dict {

class EventPublisher {
public:
    static EventPublisher& instance();

    template<typename T>
    void publish(const T& event) {
        eventSystem_.getEventBus<T>().publish(event);
    }

private:
    EventPublisher();
    EventSystem& eventSystem_;
    static constexpr const char* TAG = "EventPublisher";
};

} // namespace dict
