#pragma once

#include <sasl/common/common_fwd.h>
#include <string>

namespace sasl::common {

class diag_item;

enum compiler_compatibility { cc_msvc, cc_gcc };

std::string str(diag_item const*, compiler_compatibility cc = cc_msvc);

}  // namespace sasl::common