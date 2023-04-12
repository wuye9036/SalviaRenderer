#pragma once

#include "config.h"

#include <cstdint>

#if defined(EFLIB_MSVC) || defined(EFLIB_GCC)
#  include <source_location>
#elif defined(EFLIB_CLANG)
#  if __has_builtin(__builtin_COLUMN)
#    define EF_CLANG_BUILTIN_COLUMN() __builtin_COLUMN()
#  else
#    define EF_CLANG_BUILTIN_COLUMN() 0
#  endif
namespace std {
struct source_location { // NOLINT
  static constexpr source_location
  current(const char* f = __builtin_FILE(),
          const char* fn = __builtin_FUNCTION(),
          uint_least32_t l = __builtin_LINE(),
          uint_least32_t c = EF_CLANG_BUILTIN_COLUMN()) noexcept {
    source_location location;
    location.file_ = f;
    location.func_ = fn;
    location.line_ = l;
    location.col_ = c;
    return location;
  }

  constexpr source_location() noexcept : file_("unknown"), func_(file_), line_(0), col_(0) {}

  [[nodiscard]] constexpr uint_least32_t line() const noexcept { return line_; }
  [[nodiscard]] constexpr uint_least32_t column() const noexcept { return col_; }
  [[nodiscard]] constexpr const char* file_name() const noexcept { return file_; }
  [[nodiscard]] constexpr const char* function_name() const noexcept { return func_; }

private:
  const char* file_;
  const char* func_;
  uint_least32_t line_;
  uint_least32_t col_;
};
}  // namespace std
#else
#  error "No source_location support"
#endif