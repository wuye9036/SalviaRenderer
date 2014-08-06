
#include "./storage_mode.h"


void register_enum_name( std::function<void (char const*, storage_mode)> const& reg_fn )
{
	reg_fn("none", storage_mode::none);
	reg_fn("constant", storage_mode::constant);
	reg_fn("stack_based_address", storage_mode::stack_based_address);
	reg_fn("register_id", storage_mode::register_id);

}

