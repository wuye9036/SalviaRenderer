#include <sasl/common/diag_formatter.h>

#include <eflib/diagnostics/assert.h>
#include <sasl/common/diag_item.h>

#include <fmt/format.h>

using fmt::format;
using std::string_view;

namespace sasl::common {

std::string str(diag_item const* item, compiler_compatibility cc) {
  std::string error_level;

  switch (item->level()) {
  case diag_levels::debug: error_level = "debug"; break;
  case diag_levels::info: error_level = "info"; break;
  case diag_levels::warning: error_level = "warning"; break;
  case diag_levels::error: error_level = "error"; break;
  case diag_levels::fatal_error: error_level = "fatal error"; break;
  }

  switch (cc) {
  case cc_msvc:
    switch (item->level()) {
    case diag_levels::debug:
    case diag_levels::info:
    case diag_levels::warning:
    case diag_levels::error:
    case diag_levels::fatal_error:
      return fmt::format("{}({}): {} C{:04d}: {}",
                         item->file_name(),
                         item->span().begin.line,
                         error_level,
                         item->id(),
                         item->str());
    }

    break;
  case cc_gcc:
    ef_unimplemented();
    return std::string{item->str()};
    break;
  }

  ef_unimplemented();
  return std::string{item->str()};
}

}  // namespace sasl::common
