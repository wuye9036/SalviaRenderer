#pragma once

#include <eflib/platform/config.h>

#include <eflib/platform/source_location.h>

#include <fmt/format.h>

#include <cstdlib>
#include <iostream>

#if defined(EFLIB_MSVC)
#  define ef_debug_break() __debugbreak()
#endif

#if !defined(ef_debug_break) && defined(__clang__)
#  if __has_builtin(__builtin_debugtrap)
#    define ef_debug_break() __builtin_debugtrap()
#  endif
#endif

#if !defined(ef_debug_break) && defined(__GNUC__)
#  define ef_debug_break() __builtin_trap()
#endif

#if !defined(ef_debug_break) && __has_include(<signal.h>)
#  include <signal.h>
#  define ef_debug_break() raise(SIGTRAP)
#endif

#if !defined(ef_debug_break)
#  define ef_debug_break() ::abort()
#endif

namespace eflib {
template <class T>
void print_vector(std::ostream& os, const T& v) {
  for (typename T::const_iterator cit = v.begin(); cit != v.end(); ++cit) {
    os << *cit << " ";
  }
}

template <typename... T, typename... Args>
void report_and_break(std::source_location source_loc, std::string_view fmt, Args&&... args) {
  fmt::vprint(stderr, fmt, fmt::make_format_args(std::forward<Args>(args)...));
  fmt::print(stderr, "@ {:s}:{:d}\n", source_loc.file_name(), source_loc.line());
  ef_debug_break();
}

template <typename... T, typename... Args>
void verify(std::source_location source_loc, bool flag, std::string_view fmt, Args&&... args) {
  if (!flag) {
    report_and_break(source_loc, fmt, std::forward<Args>(args)...);
  }
}
}  // namespace eflib

#define ef_verify(expr) eflib::verify(std::source_location::current(), (expr), #expr);

#define ef_unreachable(...) eflib::report_and_break(std::source_location::current(), __VA_ARGS__)

#define ef_unimplemented() \
  eflib::report_and_break(std::source_location::current(), "The code block is unimplemented.");

#if defined(EFLIB_DEBUG)

#  define EF_ASSERT(expr, desc)                                                        \
    {                                                                                  \
      if (!(expr)) {                                                                   \
        eflib::report_and_break(                                                       \
            std::source_location::current(), "Assertion failed: {}. {}", #expr, desc); \
      }                                                                                \
    }

//  e.g.
//    EFLIB_ASSERT_AND_IF( expr, "Assert!" ){
//      return 0;
//    }
//  means
//    assert( expr && !"Assert!" );
//    if( !expr ){
//      return 0;
//    }
#  define EFLIB_ASSERT_AND_IF(expr, desc) \
    EF_ASSERT(expr, desc);                \
    if (!(expr)) /* jump statement */

#else

#  define EF_ASSERT(exp, desc, ...) (void)(exp);
#  define EFLIB_ASSERT_AND_IF(expr, desc) if (!(expr)) /* jump statement */

#endif

namespace eflib {
const bool unimplemented = false;
const bool unexpected = false;
}  // namespace eflib

#define EFLIB_ASSERT_UNIMPLEMENTED0(desc) EF_ASSERT(eflib::unimplemented, desc);
#define EFLIB_ASSERT_UNEXPECTED() \
  EF_ASSERT(eflib::unexpected, "Here is not expected to be executed.");