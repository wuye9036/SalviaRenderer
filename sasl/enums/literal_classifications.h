#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class literal_classifications: uint32_t
{
	real = UINT32_C( 4 ),
	none = UINT32_C( 1 ),
	string = UINT32_C( 5 ),
	character = UINT32_C( 6 ),
	boolean = UINT32_C( 2 ),
	integer = UINT32_C( 3 )
};

void register_enum_name( std::function<void (char const*, literal_classifications)> const& reg_fn );

