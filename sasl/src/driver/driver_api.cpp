#include <sasl/include/driver/driver_api.h>
#include <sasl/include/driver/driver_impl.h>

SASL_DRIVER_API void create_driver( boost::shared_ptr<sasl::driver::driver>& out )
{
	out = boost::shared_ptr<sasl::driver::driver>( new sasl::driver::driver_impl() );
}
