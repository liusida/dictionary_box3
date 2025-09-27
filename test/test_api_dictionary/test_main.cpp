#include "../../lib/api_dictionary/dictionary_api.h"
#include "../../lib/core_eventing/event_system.h"
#include "../../lib/core_eventing/events.h"
#include "../../lib/core_misc/memory_test_helper.h"
#include "network_control.h"
#include "test_wifi_credentials.h"
#include <Arduino.h>
#include <WiFi.h>
#include <unity.h>

using namespace dict;

// What are tested here:
// test_core.cpp
// Constructor/Destructor: DictionaryApi constructs and destructs properly
void test_dictionary_api_constructor_destructor(void);
// Initialize: DictionaryApi initializes and reports ready state
void test_dictionary_api_initialize_and_ready(void);
// Shutdown: DictionaryApi shuts down cleanly
void test_dictionary_api_shutdown(void);
// URL encoding: urlEncode() handles various input strings correctly
void test_dictionary_api_url_encoding(void);
// Word validation: isWordValid() correctly validates word inputs
void test_dictionary_api_word_validation(void);
// Configuration: setBaseUrl() and getBaseUrl() work correctly
void test_dictionary_api_configuration(void);
// Ready state: isReady() correctly reflects initialization and WiFi state
void test_dictionary_api_ready_state(void);
// Error handling: DictionaryApi handles invalid inputs gracefully
void test_dictionary_api_error_handling(void);

// test_audio_url.cpp
// Audio URL configuration: setAudioBaseUrl() and getAudioBaseUrl() work correctly
void test_dictionary_api_audio_url_configuration(void);
// Audio URL generation: getAudioUrl() generates correct URLs
void test_dictionary_api_audio_url_generation(void);
// Audio URL types: Different audio types are handled correctly
void test_dictionary_api_audio_url_types(void);
// Audio URL error handling: Error handling for audio URL generation
void test_dictionary_api_audio_url_error_handling(void);

// test_events.cpp
// Event publishing: DictionaryApi publishes appropriate events
void test_dictionary_api_event_publishing(void);
// Event lookup started: LookupStarted events are published correctly
void test_dictionary_api_event_lookup_started(void);
// Event audio requested: AudioRequested events are published correctly
void test_dictionary_api_event_audio_requested(void);
// Event multiple subscribers: Multiple event subscribers work correctly
void test_dictionary_api_event_multiple_subscribers(void);
// Event cleanup: Event system cleanup works correctly
void test_dictionary_api_event_cleanup(void);

#define TAG "DictionaryApiTest"

namespace dict {
// Global objects for the test
NetworkControl *g_network = nullptr;
} // namespace dict

// =================================== TEST SETUP ===================================

void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();

    // Clear event system
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
}

void tearDown(void) {
    // Check for memory leaks after each test
    tearDownMemoryMonitoring("test");
}

bool setupWiFi() {
    ESP_LOGI(TAG, "Setting up WiFi for testing...");

    g_network = new NetworkControl();
    g_network->initialize();
    g_network->randomizeMACAddress();

    // Ensure a clean state
    g_network->clearCredentials();

    volatile bool connectedFired = false;
    volatile bool failedFired = false;
    IPAddress connectedIp;

    g_network->setOnConnected([&](const IPAddress &ip) { connectedFired = true; });
    g_network->setOnConnectionFailed([&]() { failedFired = true; });

    TEST_ASSERT_TRUE_MESSAGE(g_network->connectToNetwork(TEST_WIFI_SSID, TEST_WIFI_PASSWORD), "WiFi.begin should be invoked successfully");

    // Loop tick until a callback fires or timeout (15s)
    unsigned long start = millis();
    while (!connectedFired && !failedFired && millis() - start < 15000) {
        g_network->tick();
        delay(100);
    }

    TEST_ASSERT_FALSE_MESSAGE(failedFired, "Connection failed callback fired");
    TEST_ASSERT_TRUE_MESSAGE(connectedFired, "Connected callback did not fire within timeout");
    if (connectedFired) {
        ESP_LOGI(TAG, "Connected. IP: %s", g_network->getIP().toString().c_str());
    }

    ESP_LOGI(TAG, "WiFi setup complete");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Setup WiFi for testing
    if (!setupWiFi()) {
        ESP_LOGE(TAG, "Failed to setup WiFi - some tests may fail");
        return;
    }

    // Print test suite memory summary
    printTestSuiteMemorySummary("DictionaryApi", true);

    UNITY_BEGIN();

    // Core Functionality Tests
    RUN_TEST_EX(TAG, test_dictionary_api_constructor_destructor);
    RUN_TEST_EX(TAG, test_dictionary_api_initialize_and_ready);
    RUN_TEST_EX(TAG, test_dictionary_api_shutdown);
    RUN_TEST_EX(TAG, test_dictionary_api_url_encoding);
    RUN_TEST_EX(TAG, test_dictionary_api_word_validation);
    RUN_TEST_EX(TAG, test_dictionary_api_configuration);
    RUN_TEST_EX(TAG, test_dictionary_api_ready_state);
    RUN_TEST_EX(TAG, test_dictionary_api_error_handling);

    // Audio URL Tests
    RUN_TEST_EX(TAG, test_dictionary_api_audio_url_configuration);
    RUN_TEST_EX(TAG, test_dictionary_api_audio_url_generation);
    RUN_TEST_EX(TAG, test_dictionary_api_audio_url_types);
    RUN_TEST_EX(TAG, test_dictionary_api_audio_url_error_handling);

    // Event System Tests
    RUN_TEST_EX(TAG, test_dictionary_api_event_publishing);
    RUN_TEST_EX(TAG, test_dictionary_api_event_lookup_started);
    RUN_TEST_EX(TAG, test_dictionary_api_event_audio_requested);
    RUN_TEST_EX(TAG, test_dictionary_api_event_multiple_subscribers);
    RUN_TEST_EX(TAG, test_dictionary_api_event_cleanup);

    UNITY_END();

    // Print test suite memory summary
    printTestSuiteMemorySummary("DictionaryApi", false);
}

void loop() {}
