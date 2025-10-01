#pragma once
#include <esp_heap_caps.h>
#include <memory>

/**
 * @brief PSRAM allocator for STL containers
 *
 * This allocator routes all allocations to PSRAM to save SRAM.
 * Use this for large STL containers that don't need fast access.
 */

template <typename T> class PsramAllocator {
public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <typename U> struct rebind {
    using other = PsramAllocator<U>;
  };

  PsramAllocator() = default;

  template <typename U> PsramAllocator(const PsramAllocator<U> &) {}

  pointer allocate(size_type n) {
    if (n == 0) {
      return nullptr;
    }

    // Try PSRAM first
    pointer ptr = static_cast<pointer>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM));
    if (ptr) {
      return ptr;
    }

    // Fallback to SRAM if PSRAM fails
    ptr = static_cast<pointer>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
    if (ptr) {
      return ptr;
    }

    // Last resort: standard malloc
    return static_cast<pointer>(malloc(n * sizeof(T)));
  }

  void deallocate(pointer p, size_type) {
    if (p) {
      // heap_caps_free handles both PSRAM and SRAM automatically
      heap_caps_free(p);
    }
  }

  bool operator==(const PsramAllocator &) const { return true; }

  bool operator!=(const PsramAllocator &) const { return false; }
};

// Convenience typedefs for common types
using PsramString = std::basic_string<char, std::char_traits<char>, PsramAllocator<char>>;
using PsramVector = std::vector<std::pair<std::string, std::string>, PsramAllocator<std::pair<std::string, std::string>>>;
