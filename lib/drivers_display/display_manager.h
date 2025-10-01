#pragma once
#include "GT911.h"
#include "TFT_eSPI.h"
#include "common.h"
#include "drivers_i2c/i2c_manager.h"
#include "lvgl.h"

namespace dict {

class DisplayManager {
public:
  // Singleton access
  static DisplayManager &instance(); // Get singleton instance

  // Core lifecycle methods
  bool initialize();    // Initialize display, touch controller, and LVGL
  void shutdown();      // Clean shutdown of display and LVGL resources
  void tick();          // Process display updates and LVGL tasks
  bool isReady() const; // Check if display is ready for use

  // Display control methods
  void resetDisplay();        // Reset display hardware and turn on backlight
  void setBacklight(bool on); // Control display backlight on/off

  // Touch handling methods
  bool initTouch();   // Initialize GT911 touch controller

  // LVGL methods
  void initLVGL();        // Initialize LVGL display and input drivers

  // Static methods
  static void dispFlushCallback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map); // LVGL display flush callback
  static void touchpadReadCallback(lv_indev_t *indev_driver, lv_indev_data_t *data);         // LVGL touch input callback
  static uint32_t tickCallback();                                                            // LVGL tick callback for timing

private:
  DisplayManager();
  ~DisplayManager() = default;
  DisplayManager(const DisplayManager &) = delete;
  DisplayManager &operator=(const DisplayManager &) = delete;
  TFT_eSPI tft_;
  GT911 touch_;
  bool displayInitialized_;
  bool touchInitialized_;
  bool lvglInitialized_;

  // LVGL objects and buffers
  lv_display_t *display_;
  lv_indev_t *inputDevice_;
  lv_color_t *buffer1_;
  lv_color_t *buffer2_;
};

} // namespace dict
