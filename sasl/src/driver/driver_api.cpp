#include <sasl/include/driver/driver_api.h>
#include <sasl/include/driver/driver_lib.h>

void sasl_create_driver( boost::shared_ptr<sasl::driver::driver>& out )
{
	out = sasl::driver::create_driver();
}
