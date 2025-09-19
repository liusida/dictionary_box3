#pragma once

#include "esp_log.h"

#define HIGHLIGHT_OUR_OWN_LOGS 0

#if HIGHLIGHT_OUR_OWN_LOGS

#ifdef ESP_LOGD
#undef ESP_LOGD
#define ESP_LOGD(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#endif

#ifdef ESP_LOGI
#undef ESP_LOGI
#define ESP_LOGI(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#endif