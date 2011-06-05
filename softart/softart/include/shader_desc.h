#ifndef SALVIA_SHADER_DESC_H
#define SALVIA_SHADER_DESC_H

#include <softart/include/shader_enums.h>

BEGIN_NS_SALVIA();

struct shader_buffer_desc{
	std::string name;
	cbuffer_type type;
	uint32_t variables;
	uint32_t size;
	uint32_t flags;
};

struct shader_variable_desc{
	std::string name;
	uint32_t start_offset;
	uint32_t size;
	uint32_t flags;
	void* default_value;
};

struct shader_type_desc{
	shader_variable_class cls;
	shader_variable_type type;
	uint32_t rows;
	uint32_t columns;
	uint32_t elements;
	uint32_t members;
	uint32_t offset;
};

struct shader_desc{
	uint32_t version;
	uint32_t flags;
	uint32_t constant_buffers;
	uint32_t bound_resources;
	uint32_t input_params;
	uint32_t output_params;
};

struct shader_param_desc{
	std::string semantic_name;
	uint32_t semantic_index;
	uint32_t reg;
	system_value_name system_value_type;
	register_component_type component_type;
	uint8_t mask;
	uint8_t read_write_mask;
	uint32_t stream;
};

struct shader_input_bind_desc {
    std::string name;
    shader_input_type type;
    uint32_t bind_point;
    uint32_t bind_count;
    uint32_t flags;
    resource_return_type return_type;
    srv_dimension dimension;
    uint32_t num_samples;
};

END_ND_SALVIA();

#endif