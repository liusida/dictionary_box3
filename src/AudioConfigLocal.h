#pragma once

// Local overrides for AudioTools that are applied before library headers.
// Switch AudioTools logging to ESP-IDF logger to avoid non-recursive mutex deadlocks
// when LOGD arguments perform nested logging.

#ifndef USE_IDF_LOGGER
#define USE_IDF_LOGGER
#endif

// Optionally, set default log level here if needed. Commented out to keep
// project defaults. You can enable Debug globally like this:
// #define USE_AUDIO_LOGGING true
// #define LOG_LEVEL AudioLogger::Debug


