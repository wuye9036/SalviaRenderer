#pragma once

#include <eflib/platform/config.h>

#include <fmt/format.h>

#include <cstdlib>
#include <iostream>
#include <source_location>

#ifdef _DEBUG

#ifdef EFLIB_MSVC
#define EFLIB_ABORT() _CrtDbgBreak();
#else
#define EFLIB_ABORT() ::abort();
#endif

#define EF_ASSERT(exp, desc)                                                                       \
  {                                                                                                \
    static bool isIgnoreAlways = false;                                                            \
    if (!isIgnoreAlways) {                                                                         \
      if ((*eflib::detail::ProcPreAssert)(exp ? true : false, #exp, desc, __LINE__, __FILE__,      \
                                          __FUNCTION__, &isIgnoreAlways)) {                        \
        EFLIB_ABORT();                                                                             \
      }                                                                                            \
    }                                                                                              \
  }

//	e.g.
//		EFLIB_ASSERT_AND_IF( expr, "Assert!" ){
//			return 0;
//		}
//	means
//		assert( expr && !"Assert!" );
//		if( !expr ){
//			return 0;
//		}
#define EFLIB_ASSERT_AND_IF(expr, desc)                                                            \
  EF_ASSERT(expr, desc);                                                                           \
  if (!(expr)) /* jump statement */

#else
#define EF_ASSERT(exp, desc) (void)(exp);
#define EFLIB_ASSERT_AND_IF(expr, desc) if (!(expr)) /* jump statement */
#endif
namespace eflib {
const bool Interrupted = false;
const bool Unimplemented = false;
const bool Unexpected = false;
} // namespace eflib

#define EFLIB_ASSERT_UNIMPLEMENTED0(desc) EF_ASSERT(eflib::Unimplemented, desc);
#define EFLIB_ASSERT_UNIMPLEMENTED()                                                               \
  EFLIB_ASSERT_UNIMPLEMENTED0(" An unimplemented code block was invoked! ");
#define EFLIB_ASSERT_UNEXPECTED()                                                                  \
  EF_ASSERT(eflib::Unexpected, "Here is not expected to be executed.");
#define EFLIB_INTERRUPT(desc) EF_ASSERT(eflib::Interrupted, desc)

namespace eflib {
namespace detail {
extern bool (*ProcPreAssert)(bool exp, const char *expstr, const char *desc, int line,
                             const char *file, const char *func, bool *ignore);

bool ProcPreAssert_Init(bool exp, const char *expstr, const char *desc, int line, const char *file,
                        const char *func, bool *ignore);
bool ProcPreAssert_Defalut(bool exp, const char *expstr, const char *desc, int line,
                           const char *file, const char *func, bool *ignore);
bool ProcPreAssert_MsgBox(bool exp, const char *expstr, const char *desc, int line,
                          const char *file, const char *func, bool *ignore);
} // namespace detail

template <class T> void print_vector(std::ostream &os, const T &v) {
  for (typename T::const_iterator cit = v.begin(); cit != v.end(); ++cit) {
    os << *cit << " ";
  }
}

template <typename... T, typename... Args>
void unreachable(std::source_location source_loc, fmt::format_string<T...> fmt, Args &&...args) {
  fmt::print(stderr, fmt, std::forward<Args>(args)...);
  fmt::print(stderr,"@ {:s}:{:d}\n", source_loc.file_name(), source_loc.line());
}
} // namespace eflib

#define ef_unreachable(...) eflib::unreachable(std::source_location::current(), __VA_ARGS__)
