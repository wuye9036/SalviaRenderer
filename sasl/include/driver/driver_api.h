#ifndef SASL_DRIVER_DRIVER_API_H
#define SASL_DRIVER_DRIVER_API_H

#include <sasl/include/driver/driver_forward.h>

#include <sasl/include/driver/driver.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

#if defined(sasl_driver_EXPORTS)
#	define SASL_DRIVER_API __declspec(dllexport)
#elif defined(SASL_STATIC_DRIVER)
#	define SASL_DRIVER_API
#else
#	define SASL_DRIVER_API __declspec(dllimport)
#endif

namespace sasl
{
	namespace driver
	{
		class driver;
	}
}

extern "C"
{
	SASL_DRIVER_API void sasl_initialize_driver();
	SASL_DRIVER_API void sasl_finalize_driver();
	SASL_DRIVER_API void sasl_create_driver( boost::shared_ptr<sasl::driver::driver>& out );
};

#endif