#include <Arduino.h>
#include <unity.h>
#include "memory_test_helper.h"

using namespace dict;

// PSRAM allocation/deallocation tests
void test_psram_malloc_free(void);
void test_psram_malloc_free_large(void);
void test_psram_malloc_free_multiple(void);

// Start Test Suite
void test_psram_malloc_free(void) {
    // Record initial memory state
    uint32_t initialHeap = ESP.getFreeHeap();
    uint32_t initialPsram = ESP.getFreePsram();
    
    ESP_LOGI("PSRAMTest", "[PSRAM Test] Initial: Heap=%u, PSRAM=%u", initialHeap, initialPsram);
    
    // Allocate buffer similar to DisplayManager (57,600 bytes)
    size_t buffer_size = 320 * 60 * sizeof(uint16_t);  // Same size as display buffers
    ESP_LOGI("PSRAMTest", "[PSRAM Test] Allocating buffer of size %zu bytes", buffer_size);
    
    void *buffer1 = ps_malloc(buffer_size);
    ESP_LOGI("PSRAMTest", "[PSRAM Test] After ps_malloc buffer1: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    void *buffer2 = ps_malloc(buffer_size);
    ESP_LOGI("PSRAMTest", "[PSRAM Test] After ps_malloc buffer2: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    TEST_ASSERT_NOT_NULL(buffer1);
    TEST_ASSERT_NOT_NULL(buffer2);
    
    // Free the buffers
    ESP_LOGI("PSRAMTest", "[PSRAM Test] Freeing buffer1...");
    free(buffer1);
    ESP_LOGI("PSRAMTest", "[PSRAM Test] After free buffer1: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    ESP_LOGI("PSRAMTest", "[PSRAM Test] Freeing buffer2...");
    free(buffer2);
    ESP_LOGI("PSRAMTest", "[PSRAM Test] After free buffer2: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    // Check for memory leaks
    bool noLeaks = checkMemoryUsage("PSRAM malloc/free", initialHeap, initialPsram);
    TEST_ASSERT_TRUE_MESSAGE(noLeaks, "Memory leak detected in PSRAM malloc/free test");
}

void test_psram_malloc_free_large(void) {
    // Record initial memory state
    uint32_t initialHeap = ESP.getFreeHeap();
    uint32_t initialPsram = ESP.getFreePsram();
    
    ESP_LOGI("PSRAMTest", "[PSRAM Large Test] Initial: Heap=%u, PSRAM=%u", initialHeap, initialPsram);
    
    // Allocate a large buffer (similar to PNG image size)
    size_t buffer_size = 320 * 240 * sizeof(uint32_t);  // 320x240 RGBA = ~300KB
    ESP_LOGI("PSRAMTest", "[PSRAM Large Test] Allocating large buffer of size %zu bytes", buffer_size);
    
    void *large_buffer = ps_malloc(buffer_size);
    ESP_LOGI("PSRAMTest", "[PSRAM Large Test] After ps_malloc large buffer: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    TEST_ASSERT_NOT_NULL(large_buffer);
    
    // Free the buffer
    ESP_LOGI("PSRAMTest", "[PSRAM Large Test] Freeing large buffer...");
    free(large_buffer);
    ESP_LOGI("PSRAMTest", "[PSRAM Large Test] After free large buffer: Heap=%u, PSRAM=%u", 
             ESP.getFreeHeap(), ESP.getFreePsram());
    
    // Check for memory leaks
    bool noLeaks = checkMemoryUsage("PSRAM large malloc/free", initialHeap, initialPsram);
    TEST_ASSERT_TRUE_MESSAGE(noLeaks, "Memory leak detected in PSRAM large malloc/free test");
}

void test_psram_malloc_free_multiple(void) {
    // Record initial memory state
    uint32_t initialHeap = ESP.getFreeHeap();
    uint32_t initialPsram = ESP.getFreePsram();
    
    ESP_LOGI("PSRAMTest", "[PSRAM Multiple Test] Initial: Heap=%u, PSRAM=%u", initialHeap, initialPsram);
    
    // Allocate multiple buffers
    const int num_buffers = 5;
    size_t buffer_size = 320 * 60 * sizeof(uint16_t);
    void *buffers[num_buffers];
    
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = ps_malloc(buffer_size);
        ESP_LOGI("PSRAMTest", "[PSRAM Multiple Test] After ps_malloc buffer %d: Heap=%u, PSRAM=%u", 
                 i+1, ESP.getFreeHeap(), ESP.getFreePsram());
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }
    
    // Free all buffers
    for (int i = 0; i < num_buffers; i++) {
        ESP_LOGI("PSRAMTest", "[PSRAM Multiple Test] Freeing buffer %d...", i+1);
        free(buffers[i]);
        ESP_LOGI("PSRAMTest", "[PSRAM Multiple Test] After free buffer %d: Heap=%u, PSRAM=%u", 
                 i+1, ESP.getFreeHeap(), ESP.getFreePsram());
    }
    
    // Check for memory leaks
    bool noLeaks = checkMemoryUsage("PSRAM multiple malloc/free", initialHeap, initialPsram);
    TEST_ASSERT_TRUE_MESSAGE(noLeaks, "Memory leak detected in PSRAM multiple malloc/free test");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    ESP_LOGI("PSRAMTest", "=== PSRAM malloc/free Tests ===");
    
    UNITY_BEGIN();
    RUN_TEST(test_psram_malloc_free);
    RUN_TEST(test_psram_malloc_free_large);
    RUN_TEST(test_psram_malloc_free_multiple);
    UNITY_END();
    
    ESP_LOGI("PSRAMTest", "=== PSRAM Tests Complete ===");
}

void loop() {}
