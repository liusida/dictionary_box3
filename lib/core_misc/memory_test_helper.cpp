#include "memory_test_helper.h"
#include "log.h"
#include "utils.h"

namespace dict {

// Global memory state for Unity setUp/tearDown
static MemoryState g_initialMemoryState = {0};

void recordMemoryState(MemoryState* state) {
    if (state == nullptr) return;
    
    state->freeHeap = ESP.getFreeHeap();
    state->freePsram = ESP.getFreePsram();
    state->minFreeHeap = ESP.getMinFreeHeap();
    state->totalHeap = ESP.getHeapSize();
    state->totalPsram = ESP.getPsramSize();
}

bool checkForMemoryLeaks(const char* testName, 
                        const MemoryState* initialState, 
                        const MemoryState* currentState,
                        uint32_t threshold) {
    if (initialState == nullptr || currentState == nullptr) {
        ESP_LOGE("MemoryTest", "Invalid memory state pointers");
        return false;
    }
    
    // Calculate signed memory delta (positive => loss, negative => gain)
    int32_t heapDelta = static_cast<int32_t>(initialState->freeHeap) - static_cast<int32_t>(currentState->freeHeap);
    int32_t psramDelta = static_cast<int32_t>(initialState->freePsram) - static_cast<int32_t>(currentState->freePsram);
    
    // Log memory status
    ESP_LOGI("MemoryTest", "=== Memory Check for %s ===", testName ? testName : "test");
    ESP_LOGI("MemoryTest", "Heap: %u -> %u (delta: %d bytes)", 
             initialState->freeHeap, currentState->freeHeap, heapDelta);
    ESP_LOGI("MemoryTest", "PSRAM: %u -> %u (delta: %d bytes)", 
             initialState->freePsram, currentState->freePsram, psramDelta);
    ESP_LOGI("MemoryTest", "Min Free Heap: %u -> %u", 
             initialState->minFreeHeap, currentState->minFreeHeap);
    
    // Check for significant memory leaks
    bool hasLeaks = false;
    if (heapDelta > static_cast<int32_t>(threshold)) {
        ESP_LOGWx("MemoryTest", "WARNING: Potential heap memory leak detected! Loss: %d bytes", heapDelta);
        hasLeaks = true;
    }
    
    if (psramDelta > static_cast<int32_t>(threshold)) {
        ESP_LOGWx("MemoryTest", "WARNING: Potential PSRAM leak detected! Loss: %d bytes", psramDelta);
        hasLeaks = true;
    }
    
    ESP_LOGI("MemoryTest", "=================================");
    
    return !hasLeaks;
}

bool checkMemoryUsage(const char* testName, 
                     uint32_t initialHeap, 
                     uint32_t initialPsram,
                     uint32_t threshold) {
    uint32_t currentHeap = ESP.getFreeHeap();
    uint32_t currentPsram = ESP.getFreePsram();
    
    int32_t heapDelta = static_cast<int32_t>(initialHeap) - static_cast<int32_t>(currentHeap);
    int32_t psramDelta = static_cast<int32_t>(initialPsram) - static_cast<int32_t>(currentPsram);
    
    ESP_LOGI("MemoryTest", "[%s] Memory: Heap %u->%u (%d), PSRAM %u->%u (%d)", 
             testName ? testName : "test", 
             initialHeap, currentHeap, heapDelta, 
             initialPsram, currentPsram, psramDelta);
    
    // Check for significant memory leaks
    bool hasLeaks = false;
    if (heapDelta > static_cast<int32_t>(threshold)) {
        ESP_LOGW("MemoryTest", "WARNING: Heap memory leak detected! Loss: %d bytes", heapDelta);
        hasLeaks = true;
    }
    
    if (psramDelta > static_cast<int32_t>(threshold)) {
        ESP_LOGW("MemoryTest", "WARNING: PSRAM leak detected! Loss: %d bytes", psramDelta);
        hasLeaks = true;
    }
    
    return !hasLeaks;
}

void setUpMemoryMonitoring() {
    recordMemoryState(&g_initialMemoryState);
}

void tearDownMemoryMonitoring(const char* testName) {
    MemoryState currentState;
    recordMemoryState(&currentState);
    
    checkForMemoryLeaks(testName, &g_initialMemoryState, &currentState);
}

void printTestSuiteMemorySummary(const char* suiteName, bool isStart) {
    if (isStart) {
        ESP_LOGI("MemoryTest", "=== Starting %s Tests ===", suiteName ? suiteName : "Test Suite");
        ESP_LOGI("MemoryTest", "Initial memory status:");
    } else {
        ESP_LOGI("MemoryTest", "=== %s Tests Complete ===", suiteName ? suiteName : "Test Suite");
        ESP_LOGI("MemoryTest", "Final memory status:");
    }
    
    printMemoryStatus();
}

} // namespace dict
