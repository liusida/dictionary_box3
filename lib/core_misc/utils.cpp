#include "utils.h"
#include "log.h"

namespace dict {

void printMemoryStatus() {
  // Internal RAM (SRAM) information
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  
  // SPIRAM (PSRAM) information
  uint32_t freePsram = ESP.getFreePsram();
  uint32_t totalPsram = ESP.getPsramSize();
  
  ESP_LOGI("Utils", "=== Memory Status ===");
  ESP_LOGI("Utils", "Internal RAM (SRAM):");
  ESP_LOGI("Utils", "  Free: %u bytes (%.2f KB)", freeHeap, freeHeap / 1024.0);
  ESP_LOGI("Utils", "  Total: %u bytes (%.2f KB)", totalHeap, totalHeap / 1024.0);
  ESP_LOGI("Utils", "  Min Free: %u bytes (%.2f KB)", minFreeHeap, minFreeHeap / 1024.0);
  ESP_LOGI("Utils", "  Used: %u bytes (%.2f KB)", totalHeap - freeHeap, (totalHeap - freeHeap) / 1024.0);
  ESP_LOGI("Utils", "  Usage: %.1f%%", ((float)(totalHeap - freeHeap) / totalHeap) * 100);
  
  if (totalPsram > 0) {
    ESP_LOGI("Utils", "SPIRAM (PSRAM):");
    ESP_LOGI("Utils", "  Free: %u bytes (%.2f KB)", freePsram, freePsram / 1024.0);
    ESP_LOGI("Utils", "  Total: %u bytes (%.2f KB)", totalPsram, totalPsram / 1024.0);
    ESP_LOGI("Utils", "  Used: %u bytes (%.2f KB)", totalPsram - freePsram, (totalPsram - freePsram) / 1024.0);
    ESP_LOGI("Utils", "  Usage: %.1f%%", ((float)(totalPsram - freePsram) / totalPsram) * 100);
  } else {
    ESP_LOGW("Utils", "SPIRAM (PSRAM): Not available");
  }
  
  // Additional system info
  ESP_LOGI("Utils", "Chip Model: %s", ESP.getChipModel());
  ESP_LOGI("Utils", "Chip Revision: %d", ESP.getChipRevision());
  ESP_LOGI("Utils", "CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
  ESP_LOGI("Utils", "Flash Size: %u bytes (%.2f MB)", ESP.getFlashChipSize(), ESP.getFlashChipSize() / (1024.0 * 1024.0));
  ESP_LOGI("Utils", "===================");
}

} // namespace dict


