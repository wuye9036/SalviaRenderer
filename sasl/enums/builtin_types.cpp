
#include "./builtin_types.h"


void register_enum_name( std::function<void (char const*, builtin_types)> const& reg_fn )
{
	reg_fn("_unsigned", builtin_types::_unsigned);
	reg_fn("_sint32", builtin_types::_sint32);
	reg_fn("_c_int", builtin_types::_c_int);
	reg_fn("_sint16", builtin_types::_sint16);
	reg_fn("_generic_type_field_shift", builtin_types::_generic_type_field_shift);
	reg_fn("_scalar_type_mask", builtin_types::_scalar_type_mask);
	reg_fn("_sign_mask", builtin_types::_sign_mask);
	reg_fn("_dim1_mask", builtin_types::_dim1_mask);
	reg_fn("_boolean", builtin_types::_boolean);
	reg_fn("_generic_type_mask", builtin_types::_generic_type_mask);
	reg_fn("_sint8", builtin_types::_sint8);
	reg_fn("_scalar", builtin_types::_scalar);
	reg_fn("_sign_field_shift", builtin_types::_sign_field_shift);
	reg_fn("_sampler", builtin_types::_sampler);
	reg_fn("_float", builtin_types::_float);
	reg_fn("_dim0_field_shift", builtin_types::_dim0_field_shift);
	reg_fn("_void", builtin_types::_void);
	reg_fn("_uint16", builtin_types::_uint16);
	reg_fn("_dimension_mask", builtin_types::_dimension_mask);
	reg_fn("_dim1_field_shift", builtin_types::_dim1_field_shift);
	reg_fn("_dimension_field_shift", builtin_types::_dimension_field_shift);
	reg_fn("_double", builtin_types::_double);
	reg_fn("_matrix", builtin_types::_matrix);
	reg_fn("_sint64", builtin_types::_sint64);
	reg_fn("_real", builtin_types::_real);
	reg_fn("_scalar_field_shift", builtin_types::_scalar_field_shift);
	reg_fn("_uint8", builtin_types::_uint8);
	reg_fn("_signed", builtin_types::_signed);
	reg_fn("_vector", builtin_types::_vector);
	reg_fn("none", builtin_types::none);
	reg_fn("_uint32", builtin_types::_uint32);
	reg_fn("_precision_field_shift", builtin_types::_precision_field_shift);
	reg_fn("_uint64", builtin_types::_uint64);
	reg_fn("_dim0_mask", builtin_types::_dim0_mask);
	reg_fn("_integer", builtin_types::_integer);

}

