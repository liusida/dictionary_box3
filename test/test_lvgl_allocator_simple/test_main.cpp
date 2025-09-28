#include <Arduino.h>
#include "lvgl_memory.h"

using namespace dict;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== LVGL Custom Allocator Test ===");
    
    // Test small allocation (should use SRAM)
    Serial.println("Testing small allocation (512 bytes) - should use SRAM");
    void* smallPtr = lvgl_malloc(512);
    if (smallPtr) {
        Serial.printf("Small allocation successful: %p\n", smallPtr);
        lvgl_free(smallPtr);
        Serial.println("Small allocation freed");
    } else {
        Serial.println("Small allocation failed!");
    }
    
    // Test large allocation (should use PSRAM)
    Serial.println("Testing large allocation (2048 bytes) - should use PSRAM");
    void* largePtr = lvgl_malloc(2048);
    if (largePtr) {
        Serial.printf("Large allocation successful: %p\n", largePtr);
        lvgl_free(largePtr);
        Serial.println("Large allocation freed");
    } else {
        Serial.println("Large allocation failed!");
    }
    
    // Test realloc
    Serial.println("Testing realloc");
    void* ptr = lvgl_malloc(256);
    if (ptr) {
        Serial.printf("Initial allocation: %p\n", ptr);
        void* newPtr = lvgl_realloc(ptr, 1024);
        if (newPtr) {
            Serial.printf("Realloc successful: %p\n", newPtr);
            lvgl_free(newPtr);
            Serial.println("Realloc freed");
        } else {
            Serial.println("Realloc failed!");
            lvgl_free(ptr);
        }
    } else {
        Serial.println("Initial allocation for realloc failed!");
    }
    
    Serial.println("=== Test Complete ===");
}

void loop() {
    // Empty
}
