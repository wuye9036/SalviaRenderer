#ifndef SASL_DRIVERS_DRIVERS_API_H
#define SASL_DRIVERS_DRIVERS_API_H

#include <sasl/include/drivers/drivers_forward.h>

#include <sasl/include/drivers/compiler.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

#if defined(sasl_drivers_EXPORTS)
#	define SASL_DRIVERS_API __declspec(dllexport)
#elif defined(SASL_STATIC_DRIVERS)
#	define SASL_DRIVERS_API
#else
#	define SASL_DRIVERS_API __declspec(dllimport)
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
	}
}

extern "C"
{
	SASL_DRIVERS_API void sasl_create_compiler	(sasl::drivers::compiler_ptr& out);
	SASL_DRIVERS_API void sasl_create_ia_shim	(sasl::shims::ia_shim_ptr&    out);
};

#endif