#include "lvgl_memory.h"
#include "core_misc/log.h"
#include <Arduino.h>

extern "C" {

static const char* TAG = "LVGL_MEM";

void* lvgl_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    void* ptr = NULL;
    
    // For small allocations, use SRAM for better performance
    if (size < LVGL_PSRAM_THRESHOLD) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
        if (ptr) {
            ESP_LOGV(TAG, "SRAM alloc: %zu bytes at %p", size, ptr);
            return ptr;
        }
        // Fallback to default malloc if SRAM allocation fails
        ESP_LOGW(TAG, "SRAM allocation failed for %zu bytes, falling back to default malloc", size);
        ptr = malloc(size);
        if (ptr) {
            ESP_LOGV(TAG, "Default malloc: %zu bytes at %p", size, ptr);
        }
        return ptr;
    }
    
    // For large allocations, try PSRAM first
    ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (ptr) {
        ESP_LOGV(TAG, "PSRAM alloc: %zu bytes at %p", size, ptr);
        return ptr;
    }
    
    // Fallback to SRAM if PSRAM allocation fails
    ESP_LOGW(TAG, "PSRAM allocation failed for %zu bytes, falling back to SRAM", size);
    ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (ptr) {
        ESP_LOGV(TAG, "SRAM fallback: %zu bytes at %p", size, ptr);
        return ptr;
    }
    
    // Last resort: default malloc
    ESP_LOGW(TAG, "All allocation methods failed for %zu bytes", size);
    ptr = malloc(size);
    if (ptr) {
        ESP_LOGV(TAG, "Default malloc fallback: %zu bytes at %p", size, ptr);
    }
    
    return ptr;
}

void lvgl_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Standard free works with both ps_malloc and heap_caps_malloc
    free(ptr);
    ESP_LOGV(TAG, "Freed: %p", ptr);
}

void* lvgl_realloc(void* ptr, size_t size) {
    if (size == 0) {
        lvgl_free(ptr);
        return NULL;
    }
    
    if (ptr == NULL) {
        return lvgl_malloc(size);
    }
    
    // Try to reallocate in the same memory type first
    void* new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
    if (new_ptr) {
        ESP_LOGV(TAG, "PSRAM realloc: %p -> %p (%zu bytes)", ptr, new_ptr, size);
        return new_ptr;
    }
    
    new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (new_ptr) {
        ESP_LOGV(TAG, "SRAM realloc: %p -> %p (%zu bytes)", ptr, new_ptr, size);
        return new_ptr;
    }
    
    // Fallback to standard realloc
    new_ptr = realloc(ptr, size);
    if (new_ptr) {
        ESP_LOGV(TAG, "Default realloc: %p -> %p (%zu bytes)", ptr, new_ptr, size);
    }
    
    return new_ptr;
}

void lv_mem_init(void) {
    // LVGL memory initialization - nothing special needed for our custom allocator
    // The allocator functions are already implemented and ready to use
    ESP_LOGI(TAG, "LVGL memory system initialized with custom allocator");
}

// Core functions that LVGL calls internally
void* lv_malloc_core(size_t size) {
    return lvgl_malloc(size);
}

void lv_free_core(void* ptr) {
    lvgl_free(ptr);
}

void* lv_realloc_core(void* ptr, size_t size) {
    return lvgl_realloc(ptr, size);
}

void lv_mem_monitor_core(lv_mem_monitor_t* monitor_p) {
    // Basic memory monitoring - fill in available information
    if (monitor_p) {
        monitor_p->total_size = ESP.getHeapSize();
        monitor_p->free_size = ESP.getFreeHeap();
        // Calculate used size
        size_t used_size = monitor_p->total_size - monitor_p->free_size;
        monitor_p->max_used = used_size; // Simplified
        monitor_p->used_pct = (used_size * 100) / monitor_p->total_size;
        monitor_p->frag_pct = 0; // Simplified - no fragmentation calculation
    }
}

} // extern "C"
