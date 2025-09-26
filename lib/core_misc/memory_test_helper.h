#pragma once
#include "common.h"
#include <unity.h>

namespace dict {

/**
 * @brief Memory leak detection helper for Unity tests
 * 
 * This library provides utilities for detecting memory leaks during testing.
 * It tracks heap and PSRAM usage before and after tests to identify potential leaks.
 */

// Memory leak detection threshold (bytes)
#define MEMORY_LEAK_THRESHOLD_DEFAULT 1024  // 1KB default threshold

/**
 * @brief Memory state tracking structure
 */
struct MemoryState {
    uint32_t freeHeap;
    uint32_t freePsram;
    uint32_t minFreeHeap;
    uint32_t totalHeap;
    uint32_t totalPsram;
};

/**
 * @brief Record current memory state
 * @param state Pointer to MemoryState structure to fill
 */
void recordMemoryState(MemoryState* state);

/**
 * @brief Check for memory leaks between two memory states
 * @param testName Name of the test for logging
 * @param initialState Initial memory state (before test)
 * @param currentState Current memory state (after test)
 * @param threshold Memory leak threshold in bytes (use MEMORY_LEAK_THRESHOLD_DEFAULT for default)
 * @return true if no significant leaks detected, false if leaks found
 */
bool checkForMemoryLeaks(const char* testName, 
                        const MemoryState* initialState, 
                        const MemoryState* currentState,
                        uint32_t threshold = MEMORY_LEAK_THRESHOLD_DEFAULT);

/**
 * @brief Simplified memory leak check using ESP32 memory functions
 * @param testName Name of the test for logging
 * @param initialHeap Initial free heap memory
 * @param initialPsram Initial free PSRAM memory
 * @param threshold Memory leak threshold in bytes
 * @return true if no significant leaks detected, false if leaks found
 */
bool checkMemoryUsage(const char* testName, 
                     uint32_t initialHeap, 
                     uint32_t initialPsram,
                     uint32_t threshold = MEMORY_LEAK_THRESHOLD_DEFAULT);

/**
 * @brief Unity test setup function for memory monitoring
 * Call this in your setUp() function
 */
void setUpMemoryMonitoring();

/**
 * @brief Unity test teardown function for memory monitoring
 * Call this in your tearDown() function
 * @param testName Name of the test (optional, can be nullptr)
 */
void tearDownMemoryMonitoring(const char* testName = nullptr);

/**
 * @brief Print test suite memory summary
 * Call this at the beginning and end of your test suite
 * @param suiteName Name of the test suite
 * @param isStart true for start, false for end
 */
void printTestSuiteMemorySummary(const char* suiteName, bool isStart);

/**
 * @brief Enhanced test runner macro with memory monitoring
 * 
 * This macro provides a complete test execution wrapper that includes:
 * - Automatic test name from function name (keeps "test_" prefix)
 * - Consistent cyan color for all tests
 * - Memory monitoring (heap and PSRAM)
 * - Memory leak detection
 * - Consistent logging format
 * 
 * @param tag Logging tag (e.g., "AudioTest", "DisplayTest")
 * @param test_function Function name to execute
 * 
 * Example usage:
 * @code
 * #define TAG "AudioTest"
 * RUN_TEST_EX(TAG, test_audio_initialize_and_ready);
 * @endcode
 * 
 * This will automatically:
 * - Use "test_audio_initialize_and_ready" as the display name
 * - Apply cyan color (1;36)
 * - Monitor memory usage
 * - Check for leaks
 */
#define RUN_TEST_EX(tag, test_function) \
    do { \
        uint32_t initialHeap = ESP.getFreeHeap(); \
        uint32_t initialPsram = ESP.getFreePsram(); \
        \
        ESP_LOGI(tag, "\033[1;36m🧪 RUNNING: %s\033[0m", #test_function); \
        ESP_LOGI(tag, "Initial Memory: Heap=%u, PSRAM=%u", initialHeap, initialPsram); \
        RUN_TEST(test_function); \
        ESP_LOGI(tag, "Final Memory: Heap=%u, PSRAM=%u", ESP.getFreeHeap(), ESP.getFreePsram()); \
        bool noLeaks = checkMemoryUsage(#test_function, initialHeap, initialPsram); \
        if (!noLeaks) { \
            ESP_LOGE(tag, "Memory leak detected in %s test", #test_function); \
        } \
    } while(0)

} // namespace dict
