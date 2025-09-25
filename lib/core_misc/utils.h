#pragma once

#include <Arduino.h>

/**
 * @brief Print detailed memory status information to Serial
 * 
 * This function displays:
 * - Internal RAM (SRAM): free, total, minimum free, used memory, and usage percentage
 * - SPIRAM (PSRAM): free, total, used memory, and usage percentage (if available)
 * - System information: chip model, revision, CPU frequency, and flash size
 */
void printMemoryStatus();

// Display reset utility moved to drivers_display/display_utils.h


