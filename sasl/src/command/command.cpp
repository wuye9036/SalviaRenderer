#include <eflib/include/platform/config.h>


#include <sasl/include/driver/driver_api.h>
#include <sasl/include/driver/driver.h>

#include <eflib/include/platform/dl_loader.h>

using eflib::dynamic_lib;
using sasl::driver::driver;
using boost::shared_ptr;

#ifdef EFLIB_WINDOWS
#	define DRIVER_EXT ".dll"
#else
#	error "Unknown OS."
#endif

#ifdef EFLIB_DEBUG
#	define DRIVER_NAME "sasl_driver_d"
#else
#	define DRIVER_NAME "sasl_driver"
#endif

int main (int argc, char **argv){
	create_driver_pfn pfn = NULL;
	shared_ptr<dynamic_lib> driver_lib = dynamic_lib::load( std::string(DRIVER_NAME) + std::string(DRIVER_EXT) );
	driver_lib->get_function( pfn, "create_driver" );
	shared_ptr<driver> drv;
	pfn(drv);

#if defined(EFLIB_DEBUG)
	system("pause");
#endif

	return 0;
}