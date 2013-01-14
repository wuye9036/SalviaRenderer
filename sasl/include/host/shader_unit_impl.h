#pragma once

#ifndef SASL_HOST_SHADER_UNIT_IMPL_H
#define SASL_HOST_SHADER_UNIT_IMPL_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/host.h>
#include <salviar/include/shader_impl.h>
#include <salviar/include/shader_unit.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace salviar
{
	struct stream_desc;
	class  stream_assembler;
}

BEGIN_NS_SASL_HOST();

typedef void (*ia_shim_func_ptr)(
	void* output_buffer,
	salviar::stream_desc const* stream_descs,
	size_t i_vert
	);

typedef void (*shader_func_ptr)(
	void const* input_stream,  void const* input_buffer,
	void*       output_stream, void*       output_buffer
	);

class vx_shader_unit_impl: public salviar::vx_shader_unit
{
public:
	vx_shader_unit_impl(
		ia_shim_func_ptr			shim_func,
		shader_func_ptr				shader_func,
		void const*					cbuffer,
		salviar::stream_desc const*	stream_descs,
		size_t						istr_size,
		size_t						obuf_size,
		size_t						ostr_size,
		size_t						output_attrs_count
		);
	
	vx_shader_unit_impl(vx_shader_unit_impl const& rhs);
	vx_shader_unit_impl& operator = (vx_shader_unit_impl const& rhs);
	salviar::vx_shader_unit_ptr clone() const;

	uint32_t output_attributes_count() const;
	uint32_t output_attribute_modifiers(size_t index) const;

	void execute(size_t ivert, void* out_data);
	void execute(size_t ivert, salviar::vs_output& out);
	
private:
	ia_shim_func_ptr			shim_func_;
	shader_func_ptr				shader_func_;
	salviar::stream_desc const*	stream_descs_;

	size_t						output_attrs_count_;	// Only used by Cpp interpolator

	void const*					buffer_data;
	std::vector<char>			stream_data;
	std::vector<char>			stream_odata;
	std::vector<char>			buffer_odata;
};

END_NS_SASL_HOST();

#endif
