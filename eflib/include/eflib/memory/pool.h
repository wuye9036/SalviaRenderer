#pragma once

#include <eflib/memory/allocator.h>
#include <eflib/memory/pointer_calc.h>
#include <eflib/memory/vls.h>
#include <eflib/platform/stdint.h>

#include <boost/mpl/if.hpp>

#include <array>
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

namespace eflib {
namespace pool {
template <typename ObjectT, int MaxCount, bool IsFreeTogether = true>
class stack_pool {
public:
  stack_pool() { this->initialize_usage(); }

  void* malloc() { return malloc_impl(); }

  void free(void* p) { free_impl(p); }

  ~stack_pool() {}

protected:
  void initialize_usage(typename std::enable_if<!IsFreeTogether>::type* dummy = 0) {
    usage[0] = false;
    memcpy(std::addressof(usage[0]), usage[0], sizeof(usage));
  }

  void* malloc_impl() {
    for (int i = 0; i < MaxCount; ++i) {
      if (usage[i] == false) {
        usage[i] = true;
        return std::addressof(data[ObjectSize * i]);
      }
    }
    return nullptr;
  }

  void free_impl(void* const p) {
    assert(is_from_pool(p));
    intptr_t pos = ((intptr_t)p - (intptr_t)std::addressof(data[0])) / ObjectSize;
    usage[pos] = false;
  }

  bool is_from_pool(void* const p) {
    intptr_t diff = (intptr_t)p - (intptr_t)std::addressof(data[0]);
    if (diff >= 0 && diff % ObjectSize == 0 && (diff / ObjectSize < MaxCount)) {
      return true;
    }
  }

  static const int ObjectSize = sizeof(ObjectT);
  std::array<bool, MaxCount> usage;
  char data[ObjectSize * MaxCount];
};

template <typename ObjectT, int MaxCount>
class stack_pool<ObjectT, MaxCount, true> {
private:
  static const size_t ObjectSize = sizeof(ObjectT);
  unsigned char data[ObjectSize * MaxCount];
  size_t usage;

public:
  stack_pool() { initialize_usage(); }

  void* malloc() { return malloc_impl(); }

  void free(void* p) { free_impl(p); }

  ~stack_pool() {}

private:
  void initialize_usage() { usage = 0; }

  void* malloc_impl() {
    assert(usage < MaxCount);
    void* ret = std::addressof(data[ObjectSize * usage]);
    ++usage;
    return ret;
  }

  void free_impl(void* const p) { return; }
};

template <typename T>
class reserved_pool {
public:
  reserved_pool() : align(0), stride(0), sz(0), cap(0), data_mem{nullptr} {}

  ~reserved_pool() {
    if (align == 1) {
      ::free(data_mem);
    } else {
      aligned_free(data_mem);
    }
  }

  void reserve(size_t capacity, size_t alignment, size_t stride = sizeof(T)) {
    if (sz == 0 && data_mem != nullptr) {
      if (stride * capacity <= this->stride * cap && align % alignment == 0) {
        this->stride = stride;
        return;
      } else {
        if (align == 1) {
          free(data_mem);
        } else {
          aligned_free(data_mem);
        }
        data_mem = nullptr;
      }
    }

    if (data_mem == nullptr) {
      align = (alignment == 0 ? 1 : alignment);
      this->stride = stride;

      if (align == 1) {
        data_mem = static_cast<T*>(::malloc(stride * capacity));
      } else {
        data_mem =
            static_cast<T*>(aligned_malloc(stride * capacity, static_cast<uint32_t>(alignment)));
      }

      sz = 0;
      cap = capacity;
      return;
    }

    assert(false);
  }

  T* alloc() {
#if defined(EFLIB_DEBUG)
    if (sz >= cap) {
      assert(false);
      return nullptr;
    }
#endif
    T* ret = advance_bytes(data_mem, stride * sz);
    ++sz;
    return ret;
  }

  void dealloc(T*) {}

  void clear() { sz = 0; }

  vls_vector_iterator<T> begin() const { return vls_vector_iterator<T>(data_mem, stride); }

  vls_vector_iterator<T> end() const {
    return vls_vector_iterator<T>(advance_bytes(data_mem, stride * sz), stride);
  }

private:
  reserved_pool(reserved_pool const&) = delete;
  reserved_pool& operator=(reserved_pool const&) = delete;

  size_t align;
  size_t stride;
  size_t sz;
  size_t cap;
  T* data_mem;
};
}  // namespace pool
}  // namespace eflib
