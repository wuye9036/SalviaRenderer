
#include "./type_qualifiers.h"


void register_enum_name( std::function<void (char const*, type_qualifiers)> const& reg_fn )
{
	reg_fn("_uniform", type_qualifiers::_uniform);
	reg_fn("none", type_qualifiers::none);

}

