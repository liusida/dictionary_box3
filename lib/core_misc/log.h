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

#define COLOR_OUR_OWN_LOGS 1
#if COLOR_OUR_OWN_LOGS

#ifndef LOG_COLOR_BLUE
#define LOG_COLOR_BLUE "\033[0;34m"
#endif


#ifndef LOG_COLOR_GREEN
#define LOG_COLOR_GREEN "\033[0;32m"
#endif

#ifndef LOG_COLOR_YELLOW
#define LOG_COLOR_YELLOW "\033[0;33m"
#endif

#ifndef LOG_COLOR_RED
#define LOG_COLOR_RED "\033[0;31m"
#endif

#ifndef LOG_COLOR_RESET
#define LOG_COLOR_RESET "\033[0m"
#endif

#define ESP_LOGDx(tag, format, ...) ESP_LOGD(tag, LOG_COLOR_BLUE format LOG_COLOR_RESET, ##__VA_ARGS__)
#define ESP_LOGIx(tag, format, ...) ESP_LOGI(tag, LOG_COLOR_GREEN format LOG_COLOR_RESET, ##__VA_ARGS__)
#define ESP_LOGWx(tag, format, ...) ESP_LOGW(tag, LOG_COLOR_YELLOW format LOG_COLOR_RESET, ##__VA_ARGS__)
#define ESP_LOGEx(tag, format, ...) ESP_LOGE(tag, LOG_COLOR_RED format LOG_COLOR_RESET, ##__VA_ARGS__)
#endif