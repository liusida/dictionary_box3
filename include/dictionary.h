#pragma once
#include <Arduino.h>

// Undefine Arduino's word macro to avoid conflicts
#ifdef word
#undef word
#endif

// Standard C++ includes that are commonly used
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>

// Project-specific Log Format
#include "log.h"
