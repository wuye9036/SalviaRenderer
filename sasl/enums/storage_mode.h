#pragma once

#include <eflib/platform/typedefs.h>
#include <eflib/utility/enum.h>
#include <functional>

enum class storage_mode: uint32_t
{
	none = UINT32_C( 0 ),
	constant = UINT32_C( 1 ),
	stack_based_address = UINT32_C( 3 ),
	register_id = UINT32_C( 2 )
};

void register_enum_name( std::function<void (char const*, storage_mode)> const& reg_fn );

