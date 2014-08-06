#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class operators: uint32_t
{
	sub_assign = UINT32_C( 8 ),
	less = UINT32_C( 19 ),
	bit_and = UINT32_C( 34 ),
	bit_or_assign = UINT32_C( 13 ),
	prefix_incr = UINT32_C( 25 ),
	logic_and = UINT32_C( 32 ),
	postfix_incr = UINT32_C( 27 ),
	lshift_assign = UINT32_C( 15 ),
	mul_assign = UINT32_C( 9 ),
	prefix_decr = UINT32_C( 26 ),
	bit_xor_assign = UINT32_C( 14 ),
	sub = UINT32_C( 2 ),
	positive = UINT32_C( 29 ),
	rshift_assign = UINT32_C( 16 ),
	negative = UINT32_C( 30 ),
	logic_not = UINT32_C( 33 ),
	add = UINT32_C( 1 ),
	right_shift = UINT32_C( 24 ),
	mul = UINT32_C( 3 ),
	bit_and_assign = UINT32_C( 12 ),
	mod_assign = UINT32_C( 11 ),
	greater = UINT32_C( 21 ),
	bit_or = UINT32_C( 35 ),
	bit_not = UINT32_C( 37 ),
	bit_xor = UINT32_C( 36 ),
	add_assign = UINT32_C( 7 ),
	mod = UINT32_C( 5 ),
	none = UINT32_C( 0 ),
	not_equal = UINT32_C( 18 ),
	logic_or = UINT32_C( 31 ),
	greater_equal = UINT32_C( 22 ),
	left_shift = UINT32_C( 23 ),
	equal = UINT32_C( 17 ),
	postfix_decr = UINT32_C( 28 ),
	div_assign = UINT32_C( 10 ),
	less_equal = UINT32_C( 20 ),
	div = UINT32_C( 4 ),
	assign = UINT32_C( 6 )
};

void register_enum_name( std::function<void (char const*, operators)> const& reg_fn );

