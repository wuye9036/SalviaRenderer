#ifndef SASL_DRIVERS_DRIVERS_API_H
#define SASL_DRIVERS_DRIVERS_API_H

#include <sasl/include/drivers/drivers_forward.h>

#include <sasl/include/drivers/compiler.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/platform/dl_sym_vis.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

#if defined(sasl_drivers_EXPORTS)
#	define SASL_DRIVERS_API EFLIB_SYM_EXPORT
#elif defined(SASL_STATIC_DRIVERS)
#	define SASL_DRIVERS_API
#else
#	define SASL_DRIVERS_API EFLIB_SYM_IMPORT
#endif

namespace sasl
{
	namespace drivers
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(compiler);
	}
	namespace shims
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
		EFLIB_DECLARE_CLASS_SHARED_PTR(interp_shim);
	}
}

extern "C"
{
	SASL_DRIVERS_API void sasl_create_compiler		(sasl::drivers::compiler_ptr&	out);
	SASL_DRIVERS_API void sasl_create_ia_shim		(sasl::shims::ia_shim_ptr&		out);
	SASL_DRIVERS_API void sasl_create_interp_shim	(sasl::shims::interp_shim_ptr&  out);
};

#endif
