#include <Arduino.h>
#include <unity.h>
#include "memory_test_helper.h"

// What are tested here:
// test_wifi_basic.cpp
// Basic functions: WiFi mode, status, MAC address, hostname
void test_wifi_basic_functions(void);
// Network scanning: WiFi.scanNetworks() functionality
void test_wifi_network_scanning(void);
// Connection management: connect/disconnect functionality
void test_wifi_connection_management(void);
// Connection timeout: timeout handling and error codes
void test_wifi_connection_timeout(void);
// Simplest test: just connect to a network
void test_wifi_simplest(void);

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
    RUN_TEST_EX(TAG, test_wifi_simplest);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("WiFi", false);
}

void loop() {}