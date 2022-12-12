#include <sasl/common/diag_formatter.h>

#include <eflib/diagnostics/assert.h>
#include <sasl/common/diag_item.h>

#include <fmt/format.h>

using fmt::format;
using std::string_view;

namespace sasl::common {

std::string str(diag_item const *item, compiler_compatibility cc) {
  std::string error_level;

  switch (item->level()) {
  case dl_info:
    error_level = "info";
    break;
  case dl_warning:
    error_level = "warning";
    break;
  case dl_error:
    error_level = "error";
    break;
  case dl_fatal_error:
    error_level = "fatal error";
    break;
  }

  switch (cc) {
  case cc_msvc:
    switch (item->level()) {
    case dl_text:
      return std::string{item->str()};
    case dl_info:
    case dl_warning:
    case dl_error:
    case dl_fatal_error:
      return fmt::format("{}({}): {} C{:04d}: {}", item->file(),
                         item->span().line_beg, error_level, item->id(),
                         item->str());
    }

    break;
  case cc_gcc:
    EFLIB_ASSERT_UNIMPLEMENTED();
    return std::string{item->str()};
    break;
  }

  EFLIB_ASSERT_UNIMPLEMENTED();
  return std::string{item->str()};
}

} // namespace sasl::common
