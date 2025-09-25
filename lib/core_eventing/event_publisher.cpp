#include "event_publisher.h"

namespace core { namespace eventing {

EventPublisher& EventPublisher::instance() {
    static EventPublisher instance;
    return instance;
}

EventPublisher::EventPublisher() : eventSystem_(EventSystem::instance()) {
}

// All typed publish methods removed; use EventPublisher::publish<T>(...) template instead.

}} // namespace core::eventing
