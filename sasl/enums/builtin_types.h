#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class builtin_types: uint32_t
{
	_unsigned = UINT32_C( 34603008 ),
	_sint32 = UINT32_C( 35848192 ),
	_c_int = UINT32_C( 35979264 ),
	_sint16 = UINT32_C( 35782656 ),
	_generic_type_field_shift = UINT32_C( 24 ),
	_scalar_type_mask = UINT32_C( 268369920 ),
	_sign_mask = UINT32_C( 267386880 ),
	_dim1_mask = UINT32_C( 255 ),
	_boolean = UINT32_C( 50331648 ),
	_generic_type_mask = UINT32_C( 251658240 ),
	_sint8 = UINT32_C( 35717120 ),
	_scalar = UINT32_C( 0 ),
	_sign_field_shift = UINT32_C( 20 ),
	_sampler = UINT32_C( 83886080 ),
	_float = UINT32_C( 16842752 ),
	_dim0_field_shift = UINT32_C( 8 ),
	_void = UINT32_C( 67108864 ),
	_uint16 = UINT32_C( 34734080 ),
	_dimension_mask = UINT32_C( 4026531840 ),
	_dim1_field_shift = UINT32_C( 0 ),
	_dimension_field_shift = UINT32_C( 28 ),
	_double = UINT32_C( 16908288 ),
	_matrix = UINT32_C( 536870912 ),
	_sint64 = UINT32_C( 35913728 ),
	_real = UINT32_C( 16777216 ),
	_scalar_field_shift = UINT32_C( 16 ),
	_uint8 = UINT32_C( 34668544 ),
	_signed = UINT32_C( 35651584 ),
	_vector = UINT32_C( 268435456 ),
	none = UINT32_C( 0 ),
	_uint32 = UINT32_C( 34799616 ),
	_precision_field_shift = UINT32_C( 16 ),
	_uint64 = UINT32_C( 34865152 ),
	_dim0_mask = UINT32_C( 65280 ),
	_integer = UINT32_C( 33554432 )
};

void register_enum_name( std::function<void (char const*, builtin_types)> const& reg_fn );

