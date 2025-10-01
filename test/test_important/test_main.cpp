#include <Arduino.h>
#include <unity.h>
#include "../../lib/core_misc/memory_test_helper.h"
#include "../../lib/core_eventing/event_system.h"
#include "../../lib/core_eventing/events.h"
#include "display_manager.h"
#include "network_control.h"
#include "test_wifi_credentials.h"

using namespace dict;

// What are tested here:
// test_dictionary_api.cpp
// DictionaryApi core functionality used in src
void test_dictionary_api_initialize_and_ready(void);
void test_dictionary_api_lookup_word(void);
void test_dictionary_api_get_audio_url(void);
void test_dictionary_api_prewarm(void);

// test_display_manager.cpp
// DisplayManager core functionality used in src
void test_display_manager_initialize_and_ready(void);
void test_display_manager_tick_safety(void);
void test_display_manager_reset_display(void);
void test_display_manager_backlight_control(void);

// test_audio_manager.cpp
// AudioManager core functionality used in src
void test_audio_manager_initialize_and_ready(void);
void test_audio_manager_tick_safety(void);
void test_audio_manager_play_stop(void);
void test_audio_manager_volume_control(void);

// test_ble_keyboard.cpp
// BLEKeyboard core functionality used in src
void test_ble_keyboard_initialize_and_ready(void);
void test_ble_keyboard_tick_safety(void);
void test_ble_keyboard_scanning_state(void);
void test_ble_keyboard_connection_state(void);

// test_network_control.cpp
// NetworkControl core functionality used in src
void test_network_control_initialize_and_ready(void);
void test_network_control_tick_safety(void);
void test_network_control_connection_state(void);
void test_network_control_credential_management(void);
void test_network_control_scanning_functionality(void);

// test_status_overlay.cpp
// StatusOverlay core functionality used in src
void test_status_overlay_initialize_and_ready(void);
void test_status_overlay_attach_detach(void);
void test_status_overlay_status_updates(void);
void test_status_overlay_position_control(void);

// test_event_system.cpp
// EventSystem core functionality used in src
void test_event_system_process_all_events(void);
void test_event_system_function_key_events(void);
void test_event_system_event_bus_management(void);

// test_lvgl_helper.cpp
// LVGL helper functions used in src
void test_lvgl_helper_key_callbacks(void);
void test_lvgl_helper_function_key_callbacks(void);

#define TAG "InterfacesUsedBySrcTest"

// Start Test Suite
void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();
}

void tearDown(void) {
    // Check for memory leaks after each test
    tearDownMemoryMonitoring("test");
}

void setup_test_wifi() {
    NetworkControl::instance().initialize();
    NetworkControl::instance().randomizeMACAddress();
    NetworkControl::instance().connectToNetwork(TEST_WIFI_SSID, TEST_WIFI_PASSWORD);
    while (!NetworkControl::instance().isConnected()) {
        NetworkControl::instance().tick();
        delay(100);
    }
    ESP_LOGI(TAG, "WiFi setup complete");
}

void teardown_test_wifi() {
    NetworkControl::instance().shutdown();
    ESP_LOGI(TAG, "WiFi teardown complete");
}

void setup_test_display() {
    DisplayManager::instance().initialize();
    ESP_LOGI(TAG, "Display setup complete");
}

void teardown_test_display() {
    DisplayManager::instance().shutdown();
    ESP_LOGI(TAG, "Display teardown complete");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("InterfacesUsedBySrc", true);
    
    UNITY_BEGIN();
    
    // Dictionary API Tests
    setup_test_wifi();
    RUN_TEST_EX(TAG, test_dictionary_api_initialize_and_ready);
    RUN_TEST_EX(TAG, test_dictionary_api_lookup_word);
    RUN_TEST_EX(TAG, test_dictionary_api_get_audio_url);
    RUN_TEST_EX(TAG, test_dictionary_api_prewarm);
    teardown_test_wifi();
    
    // Display Manager Tests
    RUN_TEST_EX(TAG, test_display_manager_initialize_and_ready);
    RUN_TEST_EX(TAG, test_display_manager_tick_safety);
    RUN_TEST_EX(TAG, test_display_manager_reset_display);
    RUN_TEST_EX(TAG, test_display_manager_backlight_control);
    
    // Audio Manager Tests
    RUN_TEST_EX(TAG, test_audio_manager_initialize_and_ready);
    RUN_TEST_EX(TAG, test_audio_manager_tick_safety);
    RUN_TEST_EX(TAG, test_audio_manager_play_stop);
    RUN_TEST_EX(TAG, test_audio_manager_volume_control);
    
    // BLE Keyboard Tests
    RUN_TEST_EX(TAG, test_ble_keyboard_initialize_and_ready);
    RUN_TEST_EX(TAG, test_ble_keyboard_tick_safety);
    RUN_TEST_EX(TAG, test_ble_keyboard_scanning_state);
    RUN_TEST_EX(TAG, test_ble_keyboard_connection_state);
    
    // Network Control Tests
    RUN_TEST_EX(TAG, test_network_control_initialize_and_ready);
    RUN_TEST_EX(TAG, test_network_control_tick_safety);
    RUN_TEST_EX(TAG, test_network_control_connection_state);
    RUN_TEST_EX(TAG, test_network_control_credential_management);
    RUN_TEST_EX(TAG, test_network_control_scanning_functionality);
    
    // Status Overlay Tests
    setup_test_display();
    RUN_TEST_EX(TAG, test_status_overlay_initialize_and_ready);
    RUN_TEST_EX(TAG, test_status_overlay_attach_detach);
    RUN_TEST_EX(TAG, test_status_overlay_status_updates);
    RUN_TEST_EX(TAG, test_status_overlay_position_control);
    teardown_test_display();
    
    // Event System Tests
    RUN_TEST_EX(TAG, test_event_system_process_all_events);
    RUN_TEST_EX(TAG, test_event_system_function_key_events);
    RUN_TEST_EX(TAG, test_event_system_event_bus_management);
    
    // LVGL Helper Tests
    RUN_TEST_EX(TAG, test_lvgl_helper_key_callbacks);
    RUN_TEST_EX(TAG, test_lvgl_helper_function_key_callbacks);

    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("InterfacesUsedBySrc", false);
}

void loop() {}
