# SRAM Optimization Plan - Dictionary v2
**Date:** 2025-09-27  
**Target:** Free >50KB SRAM to enable all libs (display, BLE keyboard, network, audio)  
**Current Issue:** ~50KB SRAM shortage when enabling all libraries

## Executive Summary

Analysis of the integrated test (`test_integrated_1/`) reveals SRAM pressure from LVGL allocations, BLE/NimBLE buffers, STL containers, and audio buffers. The plan targets >50KB SRAM savings through strategic PSRAM migration and feature optimization.

## Current Memory Usage Analysis

### Already Optimized ✅
- **LVGL Display Buffers**: Already using PSRAM (`ps_malloc`) in `display_manager.cpp`
- **JSON Documents**: Dictionary API uses PSRAM for `JsonDocument` allocations
- **UI Initialization**: Integrated test avoids full `ui_init()`, only loads `ui_Main_screen_init()`

### Major SRAM Consumers 🔍
1. **LVGL Internal Allocations**: All LVGL objects, fonts, themes use default heap (SRAM)
2. **BLE/NimBLE Buffers**: Default connection pools, MTU buffers, CCCD storage
3. **STL Containers**: `EventBus` vectors/queues, `BLEKeyboard::discoveredDevices`
4. **Audio Buffers**: AudioTools default buffer allocations
5. **FreeRTOS Stacks**: Dictionary API prewarm task (4KB stack)

## Optimization Strategy

### Phase 1: LVGL Memory Management (Target: 20-40KB SRAM)

#### 1.1 Custom LVGL Allocator
**File:** `include/lv_conf.h`
```c
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CUSTOM
#define LV_MEM_CUSTOM_INCLUDE "lvgl_memory.h"
#define LV_MEM_CUSTOM_ALLOC   lvgl_malloc
#define LV_MEM_CUSTOM_FREE    lvgl_free
#define LV_MEM_CUSTOM_REALLOC lvgl_realloc
```

**New File:** `include/lvgl_memory.h`
```c
#pragma once
#include <esp_heap_caps.h>

void* lvgl_malloc(size_t size);
void lvgl_free(void* ptr);
void* lvgl_realloc(void* ptr, size_t size);
```

**Implementation:** Route allocations >1KB to PSRAM, small allocations to SRAM

#### 1.2 Feature Optimization
**File:** `include/lv_conf.h`
```c
// Disable unused features
#define LV_USE_XML 0                    // Save ~8KB
#define LV_USE_LODEPNG 0                // Save ~12KB (or route decode buffers to PSRAM)
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN  // Reduce logging overhead

// Keep only essential fonts
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
// Disable larger fonts (22-48) unless needed
```

### Phase 2: BLE/NimBLE Optimization (Target: 10-30KB SRAM)

#### 2.1 NimBLE Memory Constraints
**File:** `platformio.ini`
```ini
build_flags = 
    -DCONFIG_BT_NIMBLE_MAX_CONNECTIONS=1
    -DCONFIG_BT_NIMBLE_MAX_CCCDS=4
    -DCONFIG_BT_NIMBLE_MAX_ATTRIBUTES=20
    -DCONFIG_BT_NIMBLE_MAX_SERVICES=8
    -DCONFIG_BT_NIMBLE_MAX_CHARACTERISTICS=16
    -DCONFIG_BT_NIMBLE_MAX_DESCRIPTORS=8
    -DCONFIG_BT_NIMBLE_MAX_MTU=247
    -DCONFIG_BT_NIMBLE_ROLE_CENTRAL_DISABLED=0
    -DCONFIG_BT_NIMBLE_ROLE_PERIPHERAL_DISABLED=1
```

#### 2.2 BLE Container Optimization
**File:** `lib/drivers_blekeyboard/ble_keyboard.h`
```cpp
#include "psram_allocator.h"

class BLEKeyboard {
private:
    std::vector<std::pair<String, String>, PsramAllocator<std::pair<String, String>>> discoveredDevices;
    // ... rest of class
};
```

### Phase 3: STL Container Migration (Target: 5-15KB SRAM)

#### 3.1 PSRAM Allocator Implementation
**New File:** `lib/core_misc/psram_allocator.h`
```cpp
#pragma once
#include <esp_heap_caps.h>
#include <memory>

template<typename T>
class PsramAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind { using other = PsramAllocator<U>; };

    PsramAllocator() = default;
    template<typename U> PsramAllocator(const PsramAllocator<U>&) {}

    pointer allocate(size_type n) {
        return static_cast<pointer>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM));
    }

    void deallocate(pointer p, size_type) {
        heap_caps_free(p);
    }

    bool operator==(const PsramAllocator&) const { return true; }
    bool operator!=(const PsramAllocator&) const { return false; }
};
```

#### 3.2 Event System Optimization
**File:** `lib/core_eventing/event_system.h`
```cpp
#include "core_misc/psram_allocator.h"

template<typename T>
class EventBus {
private:
    std::vector<Listener, PsramAllocator<Listener>> listeners_;
    std::queue<T, std::deque<T, PsramAllocator<T>>> eventQueue_;
    // ... rest of class
};
```

### Phase 4: FreeRTOS Stack Optimization (Target: 2-6KB SRAM)

#### 4.1 Dictionary API Task Reduction
**File:** `lib/api_dictionary/dictionary_api.cpp`
```cpp
// Reduce prewarm task stack from 4096 to 2048
BaseType_t result = xTaskCreatePinnedToCore(
    prewarmTask,
    "prewarm_task",
    2048,  // Reduced from 4096
    this,
    1,
    &prewarmTaskHandle_,
    0
);
```

### Phase 5: Audio Buffer Optimization (Optional: 5-15KB SRAM)

#### 5.1 AudioTools Buffer Tuning
**File:** `lib/drivers_audio/audio_manager.cpp`
```cpp
bool AudioManager::initialize() {
    // ... existing code ...
    
    // Configure smaller buffers if AudioTools version supports it
    // This may require version-specific implementation
    out.setBufferSize(1024);  // Reduce from default if possible
    
    // ... rest of initialization ...
}
```

## Implementation Priority

### High Priority (Immediate Impact)
1. **LVGL Custom Allocator** - Biggest single impact
2. **LVGL Feature Disabling** - Quick wins
3. **NimBLE Memory Constraints** - Significant BLE savings

### Medium Priority (Moderate Impact)
4. **STL Container Migration** - Systematic SRAM reduction
5. **FreeRTOS Stack Reduction** - Easy wins

### Low Priority (Fine-tuning)
6. **Audio Buffer Optimization** - May require library version updates

## Expected Results

| Optimization | SRAM Savings | Implementation Effort |
|-------------|-------------|---------------------|
| LVGL Custom Allocator | 20-40KB | Medium |
| LVGL Feature Disable | 8-20KB | Low |
| NimBLE Constraints | 10-30KB | Low |
| STL to PSRAM | 5-15KB | Medium |
| Stack Reduction | 2-6KB | Low |
| **Total** | **45-111KB** | **Mixed** |

## Risk Assessment

### Low Risk
- LVGL feature disabling (can be re-enabled if needed)
- NimBLE memory constraints (single keyboard connection sufficient)
- FreeRTOS stack reduction (monitor for stack overflow)

### Medium Risk
- LVGL custom allocator (requires testing for performance impact)
- STL container migration (ensure PSRAM allocator compatibility)

### Mitigation
- Implement optimizations incrementally
- Add memory monitoring after each phase
- Keep fallback configurations available

## Testing Strategy

1. **Memory Monitoring**: Use existing `printMemoryStatus()` after each optimization
2. **Functionality Testing**: Verify all features work with reduced SRAM
3. **Performance Testing**: Ensure PSRAM allocations don't impact UI responsiveness
4. **Stress Testing**: Long-running tests to detect memory leaks

## Success Criteria

- **Primary**: Enable all libraries (display, BLE, network, audio) without SRAM shortage
- **Secondary**: Maintain system responsiveness and stability
- **Tertiary**: Establish patterns for future SRAM optimization

## Next Steps

1. Implement LVGL custom allocator (highest impact)
2. Disable unused LVGL features
3. Apply NimBLE memory constraints
4. Test integrated functionality
5. Iterate with remaining optimizations as needed

---

**Note**: This plan provides a systematic approach to SRAM optimization while maintaining system functionality. Each phase can be implemented independently, allowing for incremental testing and validation.
