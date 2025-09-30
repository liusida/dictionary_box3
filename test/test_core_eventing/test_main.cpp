#include <Arduino.h>
#include <unity.h>
// Use the core_eventing library headers (with namespace dict)
#include "event_system.h"
#include "events.h"
#include "event_publisher.h"

using namespace dict;

// What are tested here:
// EventBus basics: subscribe/publish queues delivery; processEvents() delivers correct payload.
void test_eventbus_subscribe_publish_basic(void);
// Multiple listeners: all subscribers invoked once per event, in subscription order.
void test_eventbus_multiple_listeners_order(void);
// Unsubscribe and clear: removed listeners are not called; clear removes all.
void test_eventbus_unsubscribe(void);
// Queue isolation: events published inside a listener are delivered in the next cycle.
void test_eventbus_queue_isolation(void);
// EventSystem identity: singleton instance and same bus returned across calls.
void test_eventsystem_singleton_and_bus_instance(void);
// processAllEvents: delivers across multiple event types.
void test_eventsystem_process_all_events_multiple_types(void);
// Publisher routing: generic publish<T>() and registerEventBus<T>() deliver custom event.
void test_eventpublisher_routing(void);


using namespace dict;

static bool received_flag = false;
static int received_value = -1;

// =================================== TESTS ===================================
void test_eventbus_subscribe_publish_basic(void) {
    // Subscribe a listener, publish an int, then deliver it via processEvents().
    EventBus<int> bus;
    received_flag = false;
    received_value = -1;
    bus.subscribe([](const int &v){
        received_flag = true;
        received_value = v;
    });
    // Publish enqueues only; no immediate delivery
    bus.publish(42);
    TEST_ASSERT_FALSE(received_flag);
    // Delivery occurs on processEvents
    bus.processEvents();
    TEST_ASSERT_TRUE(received_flag);
    TEST_ASSERT_EQUAL(42, received_value);
}

void test_eventbus_multiple_listeners_order(void) {
    EventBus<int> bus;
    int calls[2] = {0, 0};
    int order[2] = {-1, -1};
    int idx = 0;
    bus.subscribe([&](const int &v){ calls[0] = v; order[idx++] = 0; });
    bus.subscribe([&](const int &v){ calls[1] = v; order[idx++] = 1; });
    bus.publish(7);
    bus.processEvents();
    TEST_ASSERT_EQUAL(7, calls[0]);
    TEST_ASSERT_EQUAL(7, calls[1]);
    TEST_ASSERT_EQUAL(0, order[0]);
    TEST_ASSERT_EQUAL(1, order[1]);
}

void test_eventbus_unsubscribe(void) {
    EventBus<int> bus;
    bool first_called = false;
    bool second_called = false;
    auto id1 = bus.subscribe([&](const int &){ first_called = true; });
    (void)id1;
    bus.subscribe([&](const int &){ second_called = true; });
    bus.unsubscribe(0);
    bus.publish(1);
    bus.processEvents();
    TEST_ASSERT_FALSE(first_called);
    TEST_ASSERT_TRUE(second_called);
}

void test_eventbus_clear_listeners(void) {
    EventBus<int> bus;
    bool called = false;
    bus.subscribe([&](const int &){ called = true; });
    bus.clear();
    bus.publish(1);
    bus.processEvents();
    TEST_ASSERT_FALSE(called);
}

void test_eventbus_queue_isolation(void) {
    EventBus<int> bus;
    int count = 0;
    bus.subscribe([&](const int &v){
        if (v == 1) {
            ++count;
            bus.publish(2); // should not deliver in this processing cycle
        } else if (v == 2) {
            ++count;
        }
    });
    bus.publish(1);
    bus.processEvents();
    TEST_ASSERT_EQUAL(1, count); // only "1" processed
    bus.processEvents();
    TEST_ASSERT_EQUAL(2, count); // then "2" processed
}

void test_eventsystem_singleton_and_bus_instance(void) {
    auto &sys1 = EventSystem::instance();
    auto &sys2 = EventSystem::instance();
    TEST_ASSERT_EQUAL_PTR(&sys1, &sys2);
    auto &busA1 = sys1.getEventBus<int>();
    auto &busA2 = sys1.getEventBus<int>();
    TEST_ASSERT_EQUAL_PTR(&busA1, &busA2);
}

void test_eventsystem_process_all_events_multiple_types(void) {
    auto &sys = EventSystem::instance();
    bool app_called = false, audio_called = false, wifi_called = false;
    sys.processAllEvents();
    TEST_ASSERT_TRUE(app_called);
    TEST_ASSERT_TRUE(audio_called);
    TEST_ASSERT_TRUE(wifi_called);
}

void test_eventpublisher_routing(void) {
    // Use a local event type to verify generic publish<T>() and registration
    struct LocalEvent { int value; };
    auto &sys = EventSystem::instance();
    int received = 0;
    sys.getEventBus<LocalEvent>().clear();
    sys.getEventBus<LocalEvent>().subscribe([&](const LocalEvent &e){ received = e.value; });
    sys.registerEventBus<LocalEvent>();
    auto &pub = EventPublisher::instance();
    pub.publish<LocalEvent>(LocalEvent{123});
    sys.processAllEvents();
    TEST_ASSERT_EQUAL(123, received);
}

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    UNITY_BEGIN();
    RUN_TEST(test_eventbus_subscribe_publish_basic);
    RUN_TEST(test_eventbus_multiple_listeners_order);
    RUN_TEST(test_eventbus_unsubscribe);
    RUN_TEST(test_eventbus_clear_listeners);
    RUN_TEST(test_eventbus_queue_isolation);
    RUN_TEST(test_eventsystem_singleton_and_bus_instance);
    RUN_TEST(test_eventsystem_process_all_events_multiple_types);
    RUN_TEST(test_eventpublisher_routing);
    UNITY_END();
}

void loop() {}