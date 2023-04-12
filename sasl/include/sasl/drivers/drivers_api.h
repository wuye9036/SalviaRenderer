#pragma once

#include <sasl/drivers/compiler.h>

#include <eflib/platform/dl_sym_vis.h>
#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <string>

#if defined(sasl_drivers_EXPORTS)
#  define SASL_DRIVERS_API EFLIB_SYM_EXPORT
#else
#  define SASL_DRIVERS_API EFLIB_SYM_IMPORT
#endif

namespace sasl {
namespace drivers {
EFLIB_DECLARE_CLASS_SHARED_PTR(compiler);
}
namespace shims {
EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
EFLIB_DECLARE_CLASS_SHARED_PTR(interp_shim);
}  // namespace shims
}  // namespace sasl

extern "C" {
SASL_DRIVERS_API void sasl_create_compiler(sasl::drivers::compiler_ptr& out);
SASL_DRIVERS_API void sasl_create_ia_shim(sasl::shims::ia_shim_ptr& out);
SASL_DRIVERS_API void sasl_create_interp_shim(sasl::shims::interp_shim_ptr& out);
};
