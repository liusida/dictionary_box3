#pragma once
#include "src/stdlib/lv_mem.h"
#include <esp_heap_caps.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Custom LVGL memory allocator that routes allocations to appropriate memory type
 *
 * Strategy:
 * - Small allocations (< 1KB): Use SRAM for performance
 * - Large allocations (>= 1KB): Use PSRAM to save SRAM
 * - Fallback to SRAM if PSRAM allocation fails
 */

// Threshold for switching between SRAM and PSRAM (1KB)
#define LVGL_PSRAM_THRESHOLD 1024

/**
 * @brief Allocate memory for LVGL
 * @param size Size in bytes to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void *lvgl_malloc(size_t size);

/**
 * @brief Free memory allocated by lvgl_malloc
 * @param ptr Pointer to memory to free
 */
void lvgl_free(void *ptr);

/**
 * @brief Reallocate memory for LVGL
 * @param ptr Pointer to existing memory (can be NULL)
 * @param size New size in bytes
 * @return Pointer to reallocated memory or NULL on failure
 */
void *lvgl_realloc(void *ptr, size_t size);

/**
 * @brief Initialize LVGL memory system
 * This function is called by LVGL when using custom memory allocator
 */
void lv_mem_init(void);

/**
 * @brief Core memory allocation function called by LVGL
 */
void *lv_malloc_core(size_t size);

/**
 * @brief Core memory free function called by LVGL
 */
void lv_free_core(void *ptr);

/**
 * @brief Core memory reallocation function called by LVGL
 */
void *lv_realloc_core(void *ptr, size_t size);

/**
 * @brief Core memory monitoring function called by LVGL
 */
void lv_mem_monitor_core(lv_mem_monitor_t *monitor_p);

#ifdef __cplusplus
}
#endif
