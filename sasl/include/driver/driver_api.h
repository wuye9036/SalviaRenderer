#ifndef SASL_DRIVER_DRIVER_API_H
#define SASL_DRIVER_DRIVER_API_H

#include <sasl/include/driver/driver_forward.h>

#include <sasl/include/driver/driver.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

#if defined(sasl_driver_EXPORTS)
#define SASL_DRIVER_API __declspec(dllexport)
#else
#define SASL_DRIVER_API __declspec(dllimport)
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
	typedef void (*create_driver_pfn)( boost::shared_ptr<sasl::driver::driver>& out );
	SASL_DRIVER_API void create_driver( boost::shared_ptr<sasl::driver::driver>& out );
};

#endif