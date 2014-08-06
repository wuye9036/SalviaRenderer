#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class jump_mode: uint32_t
{
	_return = UINT32_C( 3 ),
	none = UINT32_C( 0 ),
	_continue = UINT32_C( 2 ),
	_break = UINT32_C( 1 )
};

void register_enum_name( std::function<void (char const*, jump_mode)> const& reg_fn );

