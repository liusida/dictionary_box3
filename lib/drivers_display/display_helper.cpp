#include "display_helper.h"
#include "log.h"

namespace dict {


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
  
  ESP_LOGI("Utils", "Display reset and backlight enabled");
}

} // namespace dict
