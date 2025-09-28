#include <Arduino.h>
#include <unity.h>
#include "psram_allocator.h"
#include "utils.h"
#include "memory_test_helper.h"

using namespace dict;

#define TAG "PSRAM_ALLOCATOR"

void test_psram_vector_allocation() {
    ESP_LOGI(TAG, "Testing PSRAM vector allocation");
    
    // Create a vector that should use PSRAM
    std::vector<std::pair<std::string, std::string>, PsramAllocator<std::pair<std::string, std::string>>> devices;
    
    // Add some test data
    devices.push_back(std::make_pair("Keyboard1", "AA:BB:CC:DD:EE:FF"));
    devices.push_back(std::make_pair("Keyboard2", "11:22:33:44:55:66"));
    devices.push_back(std::make_pair("Keyboard3", "FF:EE:DD:CC:BB:AA"));
    
    TEST_ASSERT_EQUAL_MESSAGE(3, devices.size(), "Vector should contain 3 devices");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Keyboard1", devices[0].first.c_str(), "First device name should match");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("AA:BB:CC:DD:EE:FF", devices[0].second.c_str(), "First device address should match");
}

void test_psram_string_allocation() {
    ESP_LOGI(TAG, "Testing PSRAM string allocation");
    
    // Create a string that should use PSRAM
    PsramString testString;
    
    // Add some test data
    testString = "This is a test string that should be allocated in PSRAM";
    testString += " with some additional content to make it larger";
    
    TEST_ASSERT_TRUE_MESSAGE(testString.length() > 50, "String should be longer than 50 characters");
    TEST_ASSERT_TRUE_MESSAGE(testString.find("PSRAM") != std::string::npos, "String should contain 'PSRAM'");
}

void test_psram_vector_large_allocation() {
    ESP_LOGI(TAG, "Testing PSRAM vector with large allocation");
    
    // Create a large vector that should definitely use PSRAM
    std::vector<int, PsramAllocator<int>> largeVector;
    
    // Add many elements
    for (int i = 0; i < 1000; i++) {
        largeVector.push_back(i);
    }
    
    TEST_ASSERT_EQUAL_MESSAGE(1000, largeVector.size(), "Vector should contain 1000 elements");
    TEST_ASSERT_EQUAL_MESSAGE(0, largeVector[0], "First element should be 0");
    TEST_ASSERT_EQUAL_MESSAGE(999, largeVector[999], "Last element should be 999");
}

void test_psram_allocator_fallback() {
    ESP_LOGI(TAG, "Testing PSRAM allocator fallback behavior");
    
    // Test that the allocator works even if PSRAM fails
    PsramAllocator<int> allocator;
    
    // Try to allocate some memory
    int* ptr = allocator.allocate(10);
    TEST_ASSERT_NOT_NULL_MESSAGE(ptr, "Allocation should succeed");
    
    // Initialize the memory
    for (int i = 0; i < 10; i++) {
        ptr[i] = i * 2;
    }
    
    // Verify the memory
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_MESSAGE(i * 2, ptr[i], "Memory content should match");
    }
    
    // Free the memory
    allocator.deallocate(ptr, 10);
}

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
    printTestSuiteMemorySummary("PSRAM Allocator", true);
    
    UNITY_BEGIN();
    
    // Run all tests with memory monitoring and colorful output
    RUN_TEST_EX(TAG, test_psram_vector_allocation);
    RUN_TEST_EX(TAG, test_psram_string_allocation);
    RUN_TEST_EX(TAG, test_psram_vector_large_allocation);
    RUN_TEST_EX(TAG, test_psram_allocator_fallback);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("PSRAM Allocator", false);
}

void loop() {
    // Empty
}
