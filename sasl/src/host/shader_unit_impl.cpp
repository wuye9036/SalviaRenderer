#include <sasl/include/host/shader_unit_impl.h>

#include <salviar/include/stream_assembler.h>
#include <salviar/include/shader.h>

#include <eflib/include/diagnostics/assert.h>

using namespace salviar;

BEGIN_NS_SASL_HOST();

vx_shader_unit_impl::vx_shader_unit_impl(
	ia_shim_func_ptr	shim_func,
	shader_func_ptr		shader_func,
	void const*			cbuffer,
	stream_desc const*	stream_descs,
	size_t				istr_size,
	size_t				obuf_size,
	size_t				ostr_size,
	size_t				output_attrs_count
	)
	: shim_func_(shim_func)
	, shader_func_(shader_func)
	, buffer_data(cbuffer)
	, stream_descs_(stream_descs)
	, stream_data(istr_size)
	, stream_odata(ostr_size)
	, buffer_odata(obuf_size)
	, output_attrs_count_(output_attrs_count)
{
}

vx_shader_unit_impl::vx_shader_unit_impl(vx_shader_unit_impl const& rhs)
	: shim_func_	(rhs.shim_func_)
	, shader_func_	(rhs.shader_func_)
	, buffer_data	(rhs.buffer_data)
	, stream_descs_	(rhs.stream_descs_)
	, stream_data	( rhs.stream_data.size() )
	, stream_odata	( rhs.stream_odata.size() )
	, buffer_odata	( rhs.buffer_odata.size() )
{
}

vx_shader_unit_ptr vx_shader_unit_impl::clone() const
{
	return vx_shader_unit_ptr( new vx_shader_unit_impl(*this) );
}

uint32_t vx_shader_unit_impl::output_attributes_count() const
{
	return static_cast<uint32_t>(output_attrs_count_);
}

uint32_t vx_shader_unit_impl::output_attribute_modifiers(size_t /*index*/) const
{
	return vs_output::am_linear;
}

void vx_shader_unit_impl::execute(size_t ivert, void* out_data)
{
	shim_func_(&(stream_data[0]), stream_descs_, ivert);
	shader_func_(&(stream_data[0]), buffer_data, &(stream_odata[0]), out_data);
}

void vx_shader_unit_impl::execute(size_t ivert, vs_output& /*out*/)
{
	execute( ivert, &(buffer_odata[0]) );
	EFLIB_ASSERT_UNIMPLEMENTED();
}

END_NS_SASL_HOST();