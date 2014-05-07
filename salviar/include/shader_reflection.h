#ifndef SALVIAR_SHADER_REFLECTION_H
#define SALVIAR_SHADER_REFLECTION_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>

#include <eflib/include/platform/cpuinfo.h>
#include <eflib/include/string/ustring.h>
#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/enable_shared_from_this.hpp>
#include <eflib/include/platform/boost_end.h>
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
	uint32_t const sampler_class		= 5U	<< classification_field_offset;
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
	lvt_double	= ( 2U << details::scalar_field_offset )+ details::real_class,

	lvt_f32v1	= lvt_float | details::vector_flag | ( 1 << details::vector_size_field_offset ),
	lvt_f32v2	= lvt_float | details::vector_flag | ( 2 << details::vector_size_field_offset ),
	lvt_f32v3	= lvt_float | details::vector_flag | ( 3 << details::vector_size_field_offset ),
	lvt_f32v4	= lvt_float | details::vector_flag | ( 4 << details::vector_size_field_offset ),

	lvt_f32m44	= lvt_float | details::matrix_flag | ( 4 << details::vector_size_field_offset ) | (4 << details::vector_count_field_offset)
};

enum aggregation_types
{
	aggt_none,
	aggt_array
};

enum sv_usage{
	su_none = 0,

	su_stream_in,
	su_buffer_in,
	su_stream_out,
	su_buffer_out,
	
	sv_usage_count
};

struct sv_layout{
	sv_layout()
		: logical_index(0), physical_index(0)
		, offset(0)
		, size(0)
		, padding(0)
		, usage(su_none), value_type( lvt_none ), sv(sv_none)
		, agg_type(aggt_none)
	{}

	size_t total_size() const{
		return size + padding;
	}

	size_t					logical_index;
	size_t					physical_index;

	size_t					offset;
	size_t					size;
	size_t					padding;

	sv_usage				usage;
	
	language_value_types	value_type;
	aggregation_types		agg_type;				
	int						internal_type;	// Type used in SASL.

	semantic_value			sv;
};

int const PACKAGE_ELEMENT_COUNT			= 4;
int const PACKAGE_LINE_ELEMENT_COUNT	= 2;
int const SIMD_ELEMENT_COUNT			= 4;

// ! Reflection of shader.
//
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection);
class shader_reflection
{
public:
	virtual languages	get_language() const = 0;
	virtual eflib::fixed_string
						entry_name() const = 0;
	virtual std::vector<sv_layout*>
						layouts			(sv_usage usage) const = 0;
	virtual size_t		layouts_count	(sv_usage usage) const = 0;
	virtual size_t 		total_size		(sv_usage usage) const = 0;
	virtual sv_layout*	input_sv_layout	(salviar::semantic_value const&) const = 0;
	virtual sv_layout*	input_sv_layout	(eflib::fixed_string const&) const = 0;
	virtual sv_layout*	output_sv_layout(salviar::semantic_value const&) const = 0;
	virtual bool		has_position_output() const = 0;

	virtual ~shader_reflection() {}
};

END_NS_SALVIAR();

#endif