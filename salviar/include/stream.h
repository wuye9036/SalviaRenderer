#ifndef SOFTART_STREAM_H
#define SOFTART_STREAM_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/renderer_capacity.h>
#include <salviar/include/enums.h>

#include <vector>

BEGIN_NS_SALVIAR();

enum input_formats
{
	input_unknown_format = 0,
	
	input_float,
	input_float2,
	input_float3,
	input_float4,

	input_formats_count
};

enum input_classifications{
	input_per_vertex

	// TODO
	// input_per_instance
};

struct input_element_desc
{
	const char*				semantic_name;
	uint32_t				semantic_index;
	input_formats			format;
	uint32_t				input_slot;
	uint32_t				aligned_byte_offset;
	input_classifications	slot_class;

	uint32_t				instance_data_step_rate;
	
	input_element_desc()
		: semantic_name(NULL), semantic_index(0)
		, format(input_unknown_format),
		, input_slot(0), aligned_byte_offset(0)
		, slot_class(input_per_vertex), instance_data_step_rate(0)
	{}
};

typedef std::vector<input_element_desc> input_layout_decl;

END_NS_SALVIAR()

#endif