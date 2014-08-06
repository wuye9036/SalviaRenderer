#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class token_types: uint32_t
{
	_comment = UINT32_C( 7 ),
	_preprocessor = UINT32_C( 6 ),
	_operator = UINT32_C( 4 ),
	_whitespace = UINT32_C( 5 ),
	_constant = UINT32_C( 3 ),
	_newline = UINT32_C( 8 ),
	_identifier = UINT32_C( 2 ),
	_keyword = UINT32_C( 1 )
};

void register_enum_name( std::function<void (char const*, token_types)> const& reg_fn );

