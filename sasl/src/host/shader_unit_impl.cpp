#include <sasl/include/host/shader_unit_impl.h>

#include <salviar/include/stream_assembler.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader.h>

#include <eflib/include/diagnostics/assert.h>

using namespace salviar;
using namespace sasl::shims;

BEGIN_NS_SASL_HOST();

vx_shader_unit_impl::vx_shader_unit_impl(
	ia_shim_func_ptr	shim_func,
	shader_func_ptr		shader_func,
	void const*			cbuffer,
	ia_shim_data const*	data,
	size_t				istr_size,
	size_t				obuf_size,
	size_t				ostr_size,
	vso2reg_func_ptr	vso2reg_func,
	uint32_t			vso_attrs_count,
	intptr_t const*		vso_attr_offsets,
	uint32_t const*		vso_attr_types
	)
	: shim_func_(shim_func)
	, shader_func_(shader_func)
	, buffer_data(cbuffer)
	, shim_data_(*data)
	, stream_data(istr_size)
	, stream_odata(ostr_size)
	, buffer_odata(obuf_size)
	, vso2reg_func_(vso2reg_func)
	, vso_attrs_count_(vso_attrs_count)
	, vso_attr_offsets_(vso_attr_offsets)
	, vso_attr_types_(vso_attr_types)
{
}

vx_shader_unit_impl::vx_shader_unit_impl(vx_shader_unit_impl const& rhs)
	: shim_func_		(rhs.shim_func_)
	, shader_func_		(rhs.shader_func_)
	, buffer_data		(rhs.buffer_data)
	, shim_data_		(rhs.shim_data_)
	, stream_data		( rhs.stream_data.size() )
	, stream_odata		( rhs.stream_odata.size() )
	, buffer_odata		( rhs.buffer_odata.size() )
	, vso2reg_func_		(rhs.vso2reg_func_)
	, vso_attrs_count_	(rhs.vso_attrs_count_)
	, vso_attr_offsets_	(rhs.vso_attr_offsets_)
	, vso_attr_types_	(rhs.vso_attr_types_)
{
}

vx_shader_unit_ptr vx_shader_unit_impl::clone() const
{
	return vx_shader_unit_ptr( new vx_shader_unit_impl(*this) );
}

uint32_t vx_shader_unit_impl::output_attributes_count() const
{
	return static_cast<uint32_t>(vso_attrs_count_ - 1);
}

uint32_t vx_shader_unit_impl::output_attribute_modifiers(size_t /*index*/) const
{
	return vs_output::am_linear;
}

void vx_shader_unit_impl::execute(size_t ivert, void* out_data)
{
	shim_func_(&(stream_data[0]), &shim_data_, ivert);
	shader_func_(&(stream_data[0]), buffer_data, NULL /*stream output data*/, out_data);
}

void vx_shader_unit_impl::execute(size_t ivert, vs_output& out)
{
	execute( ivert, &(buffer_odata[0]) );
	vso2reg_func_(
		out.raw_data(), &(buffer_odata[0]),
		vso_attr_offsets_, vso_attr_types_, vso_attrs_count_
		);
}

END_NS_SASL_HOST();