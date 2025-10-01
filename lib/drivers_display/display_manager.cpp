#include "display_manager.h"
#include "core_misc/log.h"
#include "lvgl_helper.h"

namespace dict {

static const char *TAG = "DisplayManager";

DisplayManager::DisplayManager()
    : displayInitialized_(false), touchInitialized_(false), lvglInitialized_(false), display_(nullptr), inputDevice_(nullptr), buffer1_(nullptr),
      buffer2_(nullptr) {}

DisplayManager::~DisplayManager() { shutdown(); }

bool DisplayManager::initialize() {
  if (displayInitialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Initializing display manager...");

  resetDisplay();

  tft_.init();
  tft_.setRotation(3);
  tft_.fillScreen(TFT_WHITE);
  setBacklight(true);
  displayInitialized_ = true;

  if (!initTouch()) {
    ESP_LOGE(TAG, "Failed to initialize touch controller");
    return false;
  }

  initLVGL();

  ESP_LOGI(TAG, "Display manager initialized successfully");
  return true;
}

void DisplayManager::shutdown() {
  if (!displayInitialized_) {
    return;
  }

  ESP_LOGI(TAG, "Shutting down display manager...");

  // Clean up LVGL if it was initialized
  if (lvglInitialized_) {
    ESP_LOGI(TAG, "Cleaning up LVGL...");

    // Disable timer handling to prevent crashes during shutdown
    lv_timer_enable(false);

    // Deinitialize LVGL (this will clean up all devices and objects internally)
    lv_deinit();

    // Free buffers after LVGL cleanup
    if (buffer1_) {
      free(buffer1_);
      buffer1_ = nullptr;
    }
    if (buffer2_) {
      free(buffer2_);
      buffer2_ = nullptr;
    }
    // Clear pointers (devices are already deleted by lv_deinit)
    display_ = nullptr;
    inputDevice_ = nullptr;

    lvglInitialized_ = false;
  }

  // Reset touch controller
  if (touchInitialized_) {
    touchInitialized_ = false;
  }

  // Reset display
  displayInitialized_ = false;

  ESP_LOGI(TAG, "Display manager shutdown complete");
}

void DisplayManager::tick() {
  if (isReady()) {
    lv_timer_handler();
  }
}

bool DisplayManager::isReady() const { return displayInitialized_ && touchInitialized_ && lvglInitialized_; }

void DisplayManager::resetDisplay() {
  // Configure pins as outputs
  pinMode(TFT_MANUAL_RST, OUTPUT);
  pinMode(TFT_BL, OUTPUT);

  // Reset sequence: High -> Low (ESP-Box-3 inverted logic)
  digitalWrite(TFT_MANUAL_RST, HIGH);
  delay(10); // Short delay
  digitalWrite(TFT_MANUAL_RST, LOW);
  delay(10); // Short delay
}

void DisplayManager::setBacklight(bool on) { digitalWrite(TFT_BL, on ? HIGH : LOW); }

bool DisplayManager::initTouch() {
  if (touchInitialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Initializing GT911 touch controller...");

  if (!I2CManager::instance().isReady()) {
    ESP_LOGI(TAG, "I2C not ready, initializing...");
    if (!I2CManager::instance().initialize()) {
      ESP_LOGE(TAG, "Failed to initialize I2C for touch controller");
      return false;
    }
  }

  uint32_t i2c_freq = I2CManager::instance().getFrequency();

  bool touch_init = false;
  for (int retry = 0; retry < 5; retry++) {
    ESP_LOGI(TAG, "Touch init attempt %d/5 at %dHz...", retry + 1, i2c_freq);
    if (retry > 0) {
      delay(200);
    }
    touch_init = touch_.begin(TOUCH_INT_PIN, TOUCH_RESET_PIN, TOUCH_I2C_ADDR, i2c_freq);
    if (touch_init) {
      ESP_LOGI(TAG, "Touch initialization successful on attempt %d", retry + 1);
      break;
    } else {
      ESP_LOGW(TAG, "Touch init attempt %d failed", retry + 1);
    }
    delay(500);
  }

  if (touch_init) {
    ESP_LOGI(TAG, "Touch init successful");
    GTInfo *info = touch_.readInfo();
    if (info) {
      ESP_LOGI(TAG, "Touch resolution: %dx%d", info->xResolution, info->yResolution);
      ESP_LOGI(TAG, "Product ID: %.4s", info->productId);
    }
    touchInitialized_ = true;
    return true;
  } else {
    ESP_LOGE(TAG, "Touch init FAILED after 5 attempts!");
    return false;
  }
}

void DisplayManager::initLVGL() {
  if (lvglInitialized_) {
    return;
  }

  ESP_LOGI(TAG, "Setting up LVGL display driver...");

  lv_init();

  tft_.setSwapBytes(true);

  ESP_LOGD(TAG, "Setting tick callback...");
  lv_tick_set_cb(tickCallback);

#define BUF_ROWS 60
  size_t buffer_size = 320 * BUF_ROWS * sizeof(lv_color_t);
  ESP_LOGI(TAG, "Allocating buffers of size %zu bytes each", buffer_size);

  // Allocate buffers for this instance
  buffer1_ = (lv_color_t *)ps_malloc(buffer_size);
  buffer2_ = (lv_color_t *)ps_malloc(buffer_size);

  if (!buffer1_ || !buffer2_) {
    ESP_LOGE(TAG, "Failed to allocate buffers in SPIRAM! buf1=%p, buf2=%p", (void *)buffer1_, (void *)buffer2_);
    if (buffer1_)
      free(buffer1_);
    if (buffer2_)
      free(buffer2_);
    buffer1_ = nullptr;
    buffer2_ = nullptr;
    return;
  }
  ESP_LOGI(TAG, "Buffers allocated successfully: buf1=%p, buf2=%p", (void *)buffer1_, (void *)buffer2_);

  display_ = lv_display_create(320, 240);
  if (display_ == NULL) {
    ESP_LOGE(TAG, "Display creation failed!");
    free(buffer1_);
    free(buffer2_);
    buffer1_ = nullptr;
    buffer2_ = nullptr;
    return;
  }

  lv_display_set_flush_cb(display_, dispFlushCallback);
  lv_display_set_user_data(display_, &tft_);

  lv_display_set_default(display_);

  lv_display_set_buffers(display_, buffer1_, buffer2_, 320 * BUF_ROWS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

  ESP_LOGI(TAG, "Setting up LVGL input driver...");

  inputDevice_ = lv_indev_create();
  lv_indev_set_type(inputDevice_, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(inputDevice_, touchpadReadCallback);
  lv_indev_set_user_data(inputDevice_, &touch_);

  lv_indev_set_display(inputDevice_, display_);

  ESP_LOGI(TAG, "LVGL display system initialized successfully!");
  ESP_LOGI(TAG, "Input driver linked to display");

  lvglInitialized_ = true;
}

void DisplayManager::dispFlushCallback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  TFT_eSPI *tft = (TFT_eSPI *)lv_display_get_user_data(disp);
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->pushPixels((uint16_t *)px_map, w * h);
  tft->endWrite();

  lv_display_flush_ready(disp);
}

void DisplayManager::touchpadReadCallback(lv_indev_t *indev_driver, lv_indev_data_t *data) {
  static uint32_t callback_count = 0;
  static uint32_t error_count = 0;
  static uint32_t last_error_time = 0;
  callback_count++;

  if (callback_count % 1000 == 0) {
    ESP_LOGD(TAG, "Touch callback called %d times, errors: %d", callback_count, error_count);
  }

  GT911 *touch = (GT911 *)lv_indev_get_user_data(indev_driver);

  bool touch_success = false;
  try {
    if (touch->touched(GT911_MODE_INTERRUPT)) {
      GTPoint *tp = touch->getPoints();
      if (tp != nullptr) {
        uint16_t x = tp[0].x;
        uint16_t y = tp[0].y;
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        touch_success = true;
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
    error_count++;
    uint32_t now = millis();
    if (now - last_error_time > 5000) {
      ESP_LOGW(TAG, "Touch I2C error #%d (suppressing further errors for 5s)", error_count);
      last_error_time = now;
    }
  }

  if (!touch_success) {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

uint32_t DisplayManager::tickCallback() { return millis(); }

} // namespace dict
