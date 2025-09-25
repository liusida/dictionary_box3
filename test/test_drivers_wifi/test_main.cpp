#include <Arduino.h>
#include <unity.h>
#include "memory_test_helper.h"

using namespace dict;

// What are tested here:
// test_wifi_basic.cpp
// Simplest test: just connect to a network and disconnect
void test_wifi_simplest(void);

// test_network_control.cpp
void test_save_and_has_saved_credentials(void);
void test_load_credentials(void);
void test_clear_credentials(void);
void test_network_control_connect_and_report_ip(void);

#define TAG "WiFiTest"

// Start Test Suite
void setUp(void) {
    // Record memory state before each test
    setUpMemoryMonitoring();
}

void tearDown(void) {
    // Check for memory leaks after each test
    tearDownMemoryMonitoring("test");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("WiFi", true);
    
    UNITY_BEGIN();
    
    // Run all tests with memory monitoring and colorful output
    // RUN_TEST_EX(TAG, test_wifi_basic_functions);
    // RUN_TEST_EX(TAG, test_wifi_network_scanning);
    // RUN_TEST_EX(TAG, test_wifi_connection_management);
    // RUN_TEST_EX(TAG, test_wifi_connection_timeout);
    // RUN_TEST_EX(TAG, test_wifi_simplest);
    RUN_TEST_EX(TAG, test_save_and_has_saved_credentials);
    RUN_TEST_EX(TAG, test_load_credentials);
    RUN_TEST_EX(TAG, test_clear_credentials);
    RUN_TEST_EX(TAG, test_network_control_connect_and_report_ip);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("WiFi", false);
}

void loop() {}