
#include "./jump_mode.h"


void register_enum_name( std::function<void (char const*, jump_mode)> const& reg_fn )
{
	reg_fn("_return", jump_mode::_return);
	reg_fn("none", jump_mode::none);
	reg_fn("_continue", jump_mode::_continue);
	reg_fn("_break", jump_mode::_break);

}

