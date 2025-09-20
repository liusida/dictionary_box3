#include "display.h"
#include "core/log.h"
static const char *TAG = "Display";

// LVGL callback functions for LVGL 9.x
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  TFT_eSPI *tft = (TFT_eSPI *)lv_display_get_user_data(disp);
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->pushPixels((uint16_t *)px_map, w * h);
  tft->endWrite();

  lv_display_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
  static uint32_t callback_count = 0;
  static uint32_t error_count = 0;
  static uint32_t last_error_time = 0;
  callback_count++;

  // Debug: Show that callback is being called
  if (callback_count % 1000 == 0) {
    ESP_LOGD(TAG, "Touch callback called %d times, errors: %d", callback_count, error_count);
  }

  GT911 *touch = (GT911 *)lv_indev_get_user_data(indev_driver);

  // Add error handling for I2C communication
  bool touch_success = false;
  try {
    // Use GT911_MODE_INTERRUPT like the working example
    if (touch->touched(GT911_MODE_INTERRUPT)) {
      // Get touch points array like the working example
      GTPoint *tp = touch->getPoints();

      if (tp != nullptr) {
        // Use first touch point
        uint16_t x = tp[0].x;
        uint16_t y = tp[0].y;

        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        touch_success = true;

        // Debug output (throttled to avoid spam)
        static uint32_t last_debug_time = 0;
        uint32_t now = millis();
        if (now - last_debug_time > 100) {
          ESP_LOGV(TAG, "Touch detected: Point: (%d, %d)", x, y);
          last_debug_time = now;
        }
      }
    } else {
      data->state = LV_INDEV_STATE_RELEASED;
      touch_success = true;
    }
  } catch (...) {
    // Catch any exceptions from I2C communication
    error_count++;
    uint32_t now = millis();
    if (now - last_error_time > 5000) { // Only log every 5 seconds
      ESP_LOGW(TAG, "Touch I2C error #%d (suppressing further errors for 5s)", error_count);
      last_error_time = now;
    }
  }

  // If touch reading failed, set released state
  if (!touch_success) {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Tick callback function for LVGL (like the working example)
static uint32_t my_tick(void) { return millis(); }

bool initTouch(GT911 &touch) {
  ESP_LOGI(TAG, "Initializing GT911 touch controller...");

  // Initialize I2C for touch controller
  // Note: Audio manager may have already initialized I2C, but Wire.begin() is safe to call multiple times
  ESP_LOGI(TAG, "Initializing I2C for touch controller...");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Start with lower I2C frequency for better stability
  uint32_t i2c_freq = 50000; // Start with 50kHz instead of 100kHz
  Wire.setClock(i2c_freq);

  // Add pull-up resistors for better I2C stability
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);

  // Try touch initialization with retry
  bool touch_init = false;
  for (int retry = 0; retry < 5; retry++) {
    ESP_LOGI(TAG, "Touch init attempt %d/5 at %dHz...", retry + 1, i2c_freq);
    
    // Add a small delay before each attempt
    if (retry > 0) {
      delay(200);
    }
    
    touch_init =
        touch.begin(TOUCH_INT_PIN, TOUCH_RESET_PIN, TOUCH_I2C_ADDR, i2c_freq);
    if (touch_init) {
      ESP_LOGI(TAG, "Touch initialization successful on attempt %d", retry + 1);
      break;
    } else {
      ESP_LOGW(TAG, "Touch init attempt %d failed", retry + 1);
    }
    delay(500); // Wait longer before retry
  }

  if (touch_init) {
    ESP_LOGI(TAG, "Touch init successful");

    // Read touch info for debugging
    GTInfo *info = touch.readInfo();
    if (info) {
      ESP_LOGI(TAG, "Touch resolution: %dx%d", info->xResolution, info->yResolution);
      ESP_LOGI(TAG, "Product ID: %.4s", info->productId);
    }
    return true;
  } else {
    ESP_LOGE(TAG, "Touch init FAILED after 5 attempts!");
    return false;
  }
}

void initLVGLDisplay(TFT_eSPI &tft, GT911 &touch) {
  ESP_LOGI(TAG, "Setting up LVGL display driver...");

  tft.setSwapBytes(true); // Replaces LV_COLOR_16_SWAP

  // Set tick callback like the working example
  ESP_LOGD(TAG, "Setting tick callback...");
  lv_tick_set_cb(my_tick);

// Allocate double buffers in SPIRAM (PSRAM) like the working example
#define BUF_ROWS 120
  static lv_color_t *buf1 =
      (lv_color_t *)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));
  static lv_color_t *buf2 =
      (lv_color_t *)ps_malloc(320 * BUF_ROWS * sizeof(lv_color_t));

  if (!buf1 || !buf2) {
    ESP_LOGE(TAG, "Failed to allocate buffers in SPIRAM!");
    return;
  }
  ESP_LOGI(TAG, "Buffers allocated: buf1=%p, buf2=%p", (void *)buf1, (void *)buf2);

  // Create display with LVGL 9.x API
  lv_display_t *disp = lv_display_create(320, 240);
  if (disp == NULL) {
    ESP_LOGE(TAG, "Display creation failed!");
    return;
  }

  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_user_data(disp, &tft);

  // Use partial rendering mode like the working example
  lv_display_set_buffers(disp, buf1, buf2, 320 * BUF_ROWS * sizeof(lv_color_t),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  ESP_LOGI(TAG, "Setting up LVGL input driver...");

  // Set up LVGL input driver (touch) with LVGL 9.x API
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_indev_set_user_data(indev, &touch);

  // Link input device to display (like the working example)
  lv_indev_set_display(indev, disp);

  ESP_LOGI(TAG, "LVGL display system initialized successfully!");
  ESP_LOGI(TAG, "Input driver linked to display");
}

void handleLVGLTasks() { lv_timer_handler(); }
