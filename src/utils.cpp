#include "utils.h"

void printMemoryStatus() {
  // Internal RAM (SRAM) information
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  
  // SPIRAM (PSRAM) information
  uint32_t freePsram = ESP.getFreePsram();
  uint32_t totalPsram = ESP.getPsramSize();
  
  Serial.println("=== Memory Status ===");
  Serial.printf("Internal RAM (SRAM):\n");
  Serial.printf("  Free: %u bytes (%.2f KB)\n", freeHeap, freeHeap / 1024.0);
  Serial.printf("  Total: %u bytes (%.2f KB)\n", totalHeap, totalHeap / 1024.0);
  Serial.printf("  Min Free: %u bytes (%.2f KB)\n", minFreeHeap, minFreeHeap / 1024.0);
  Serial.printf("  Used: %u bytes (%.2f KB)\n", totalHeap - freeHeap, (totalHeap - freeHeap) / 1024.0);
  Serial.printf("  Usage: %.1f%%\n", ((float)(totalHeap - freeHeap) / totalHeap) * 100);
  
  if (totalPsram > 0) {
    Serial.printf("SPIRAM (PSRAM):\n");
    Serial.printf("  Free: %u bytes (%.2f KB)\n", freePsram, freePsram / 1024.0);
    Serial.printf("  Total: %u bytes (%.2f KB)\n", totalPsram, totalPsram / 1024.0);
    Serial.printf("  Used: %u bytes (%.2f KB)\n", totalPsram - freePsram, (totalPsram - freePsram) / 1024.0);
    Serial.printf("  Usage: %.1f%%\n", ((float)(totalPsram - freePsram) / totalPsram) * 100);
  } else {
    Serial.println("SPIRAM (PSRAM): Not available");
  }
  
  // Additional system info
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %u bytes (%.2f MB)\n", ESP.getFlashChipSize(), ESP.getFlashChipSize() / (1024.0 * 1024.0));
  Serial.println("===================");
}

void manualResetDisplay() {
  // Configure pins as outputs
  pinMode(TFT_MANUAL_RST, OUTPUT);
  pinMode(TFT_BL, OUTPUT);
  
  // Reset sequence: High -> Low (ESP-Box-3 inverted logic)
  digitalWrite(TFT_MANUAL_RST, HIGH);
  delay(10); // Short delay
  digitalWrite(TFT_MANUAL_RST, LOW);
  delay(10); // Short delay
  
  // Turn on backlight
  digitalWrite(TFT_BL, HIGH);
  
  Serial.println("Display reset and backlight enabled");
}
