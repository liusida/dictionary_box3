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

/**
 * @brief Manually reset the display and turn on backlight
 * 
 * This function:
 * - Sets TFT_MANUAL_RST high, then low to reset the display
 * - Sets TFT_BL high to turn on the backlight
 */
void manualResetDisplay();

