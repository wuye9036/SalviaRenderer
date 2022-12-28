#include <eflib/diagnostics/assert.h>

namespace eflib {
namespace detail {
void do_init() {
// All eflib initialization code is at here.

// initialize debug helper
#ifndef EFLIB_WINDOWS
  eflib::detail::ProcPreAssert = &eflib::detail::default_pre_assert;
#else
  eflib::detail::ProcPreAssert = &eflib::detail::ProcPreAssert_MsgBox;
#endif
}
} // namespace detail
} // namespace eflib