#include <sasl/include/driver/driver_lib.h>
#include <sasl/include/driver/driver_impl.h>

using boost::shared_ptr;

BEGIN_NS_SASL_DRIVER();
shared_ptr<driver> create_driver()
{
	return shared_ptr<driver>( new driver_impl() );
}
END_NS_SASL_DRIVER();