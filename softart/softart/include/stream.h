#ifndef SOFTART_STREAM_H
#define SOFTART_STREAM_H

#include "renderer_capacity.h"
#include "enums.h"

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


enum stream_index
{
	stream_0 = 0,
	stream_1 = 1,
	stream_2 = 2,
	stream_3 = 3,
	stream_4 = 4,
	stream_5 = 5,
	stream_6 = 6,
	stream_7 = 7,
	stream_index_count = 8
};

enum input_register_index
{
	input_reg_0 = 0,
	input_reg_1 = 1,
	input_reg_2 = 2,
	input_reg_3 = 3,
	input_register_index_count = 4
};

enum input_type
{
	input_float = 0,
	input_float2 = 1,
	input_float3 = 2,
	input_float4 = 3,
	input_type_count = 4
};

struct input_element_decl
{
	size_t stream_idx;
	size_t offset;
	size_t stride;
	input_type type;
	input_register_usage_decl usage;
	size_t regidx;
	
	input_element_decl(){}

	input_element_decl(
		stream_index sidx,
		size_t offset,
		size_t stride,
		input_type type,
		input_register_usage_decl usage,
		input_register_index regidx
		)
		:stream_idx(sidx), offset(offset), stride(stride), type(type), usage(usage), regidx(regidx)
	{}
};

typedef std::vector<input_element_decl> input_layout_decl;

END_NS_SOFTART()

#endif