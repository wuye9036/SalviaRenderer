#include <sasl/drivers/compiler_lib.h>
#include <sasl/drivers/compiler_impl.h>

using std::shared_ptr;

namespace sasl::drivers {
shared_ptr<compiler> create_compiler()
{
	return shared_ptr<compiler>( new compiler_impl() );
}
}