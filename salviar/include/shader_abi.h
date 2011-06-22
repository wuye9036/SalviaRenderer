#ifndef SALVIAR_SHADER_ABI_H
#define SALVIAR_SHADER_ABI_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>

#include <vector>

BEGIN_NS_SALVIAR();

#define SALVIA_LVT_VECTOR_OF( scalar, length )
#define SALVIA_LVT_MATRIX_OF( scalar, vector_size, vector_count )

namespace details{
	uint32_t const dimension_field_offset		= 28;
	uint32_t const classification_field_offset	= 24;
	uint32_t const sign_field_offset			= 20;
	uint32_t const precision_field_offset		= 16;
	uint32_t const scalar_field_offset			= 16;
	uint32_t const vector_size_field_offset		= 8;
	uint32_t const vector_count_field_offset	= 0;

	uint32_t const dimension_mask		= 0xFU	<< dimension_field_offset;
	uint32_t const scalar_flag			= 0U	<< dimension_field_offset;
	uint32_t const vector_flag			= 1U	<< dimension_field_offset;
	uint32_t const matrix_flag			= 2U	<< dimension_field_offset;

	uint32_t const scalar_mask			= ( 1U << dimension_field_offset ) - ( 1U << scalar_field_offset );
	uint32_t const vector_size_mask		= 0xFF00U << vector_size_field_offset;
	uint32_t const vector_count_mask	= 0xFFU;

	uint32_t const real_class			= 1U	<< classification_field_offset;
	uint32_t const integer_class		= 2U	<< classification_field_offset;
	uint32_t const boolean_class		= 3U	<< classification_field_offset;
	uint32_t const void_class			= 4U	<< classification_field_offset;
	uint32_t const classification_mask	= 0xFU	<< classification_field_offset;

	uint32_t const signed_flag			= ( 1U << sign_field_offset ) + integer_class;
	uint32_t const unsigned_flag		= ( 2U << sign_field_offset ) + integer_class;
	uint32_t const sign_mask			= 0xFFU	<< sign_field_offset;
}

enum language_value_types
{
	lvt_none	= 0,

	lvt_void	= details::void_class,
	lvt_boolean	= details::boolean_class,

	lvt_sint8	= ( 1U << details::scalar_field_offset) + details::signed_flag ,
	lvt_sint16	= ( 2U << details::scalar_field_offset) + details::signed_flag ,
	lvt_sint32	= ( 3U << details::scalar_field_offset) + details::signed_flag ,
	lvt_sint64	= ( 4U << details::scalar_field_offset) + details::signed_flag ,

	lvt_uint8	= ( 1U << details::scalar_field_offset) + details::unsigned_flag ,
	lvt_uint16	= ( 2U << details::scalar_field_offset) + details::unsigned_flag ,
	lvt_uint32	= ( 3U << details::scalar_field_offset) + details::unsigned_flag ,
	lvt_uint64	= ( 4U << details::scalar_field_offset) + details::unsigned_flag ,

	lvt_float	= ( 1U << details::scalar_field_offset )+ details::real_class,
	lvt_double	= ( 2U << details::scalar_field_offset )+ details::real_class
};

enum storage_classifications{
	sc_none = 0,

	sc_stream_in,
	sc_stream_out,
	sc_buffer_in,
	sc_buffer_out,
	
	storage_classifications_count
};

struct storage_info{
	storage_info()
		: index(-1), offset(0), size(0)
		, storage(sc_none), value_type( lvt_none ), sv(sv_none)
	{}

	int						index;
	int						offset;
	int						size;
	storage_classifications	storage;
	language_value_types	value_type;
	semantic_value			sv;
};

// ! Application binary interface of shader.
//
class shader_abi{
	virtual std::string entry_name() const = 0;

	virtual std::vector<storage_info*> storage_infos( storage_classifications sclass ) = 0;
	virtual size_t total_size( storage_classifications sclass ) = 0;

	virtual storage_info* input_storage( salviar::semantic_value const& ) = 0;
	virtual storage_info* input_storage( std::string const& ) = 0;

	virtual storage_info* output_storage( salviar::semantic_value const& ) = 0;
};

END_NS_SALVIAR();

#endif