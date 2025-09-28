#include <Arduino.h>
#include <unity.h>
#include "lvgl_memory.h"
#include "utils.h"
#include "memory_test_helper.h"

using namespace dict;

#define TAG "LVGL_ALLOCATOR"

void test_lvgl_small_allocation() {
    ESP_LOGI(TAG, "Testing small allocation (< 1KB) - should use SRAM");
    
    // Allocate small buffer (should go to SRAM)
    void* ptr = lvgl_malloc(512);
    TEST_ASSERT_NOT_NULL_MESSAGE(ptr, "Small allocation should succeed");
    
    lvgl_free(ptr);
}

void test_lvgl_large_allocation() {
    ESP_LOGI(TAG, "Testing large allocation (>= 1KB) - should use PSRAM");
    
    // Allocate large buffer (should go to PSRAM)
    void* ptr = lvgl_malloc(2048);
    TEST_ASSERT_NOT_NULL_MESSAGE(ptr, "Large allocation should succeed");
    
    lvgl_free(ptr);
}

void test_lvgl_realloc() {
    ESP_LOGI(TAG, "Testing realloc functionality");
    
    // Start with small allocation
    void* ptr = lvgl_malloc(512);
    TEST_ASSERT_NOT_NULL_MESSAGE(ptr, "Initial allocation should succeed");
    
    // Realloc to large size (should move to PSRAM)
    void* newPtr = lvgl_realloc(ptr, 2048);
    TEST_ASSERT_NOT_NULL_MESSAGE(newPtr, "Realloc should succeed");
    
    lvgl_free(newPtr);
}

void test_lvgl_multiple_allocations() {
    ESP_LOGI(TAG, "Testing multiple allocations of different sizes");
    
    void* smallPtrs[5];
    void* largePtrs[3];
    
    // Allocate multiple small buffers (SRAM)
    for (int i = 0; i < 5; i++) {
        smallPtrs[i] = lvgl_malloc(256);
        TEST_ASSERT_NOT_NULL_MESSAGE(smallPtrs[i], "Small allocation should succeed");
    }
    
    // Allocate multiple large buffers (PSRAM)
    for (int i = 0; i < 3; i++) {
        largePtrs[i] = lvgl_malloc(1024);
        TEST_ASSERT_NOT_NULL_MESSAGE(largePtrs[i], "Large allocation should succeed");
    }
    
    // Free all allocations
    for (int i = 0; i < 5; i++) {
        lvgl_free(smallPtrs[i]);
    }
    for (int i = 0; i < 3; i++) {
        lvgl_free(largePtrs[i]);
    }
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
    printTestSuiteMemorySummary("LVGL Allocator", true);
    
    UNITY_BEGIN();
    
    // Run all tests with memory monitoring and colorful output
    RUN_TEST_EX(TAG, test_lvgl_small_allocation);
    RUN_TEST_EX(TAG, test_lvgl_large_allocation);
    RUN_TEST_EX(TAG, test_lvgl_realloc);
    RUN_TEST_EX(TAG, test_lvgl_multiple_allocations);
    
    UNITY_END();
    
    // Print test suite memory summary
    printTestSuiteMemorySummary("LVGL Allocator", false);
}

void loop() {
    // Empty
}
