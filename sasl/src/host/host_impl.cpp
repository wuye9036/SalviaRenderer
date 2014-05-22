#include <sasl/include/host/host_impl.h>

#include <sasl/include/shims/ia_shim.h>
#include <sasl/include/shims/interp_shim.h>

#include <sasl/include/host/shader_object_impl.h>
#include <sasl/include/host/shader_log_impl.h>
#include <sasl/include/host/shader_unit_impl.h>

#include <sasl/include/common/diag_chat.h>
#include <sasl/include/common/diag_item.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/drivers/drivers_api.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/semantic/reflection_impl.h>

#include <salviar/include/shader_reflection.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/render_state.h>
#include <salviar/include/render_stages.h>
#include <salviar/include/shader_cbuffer.h>

#include <eflib/include/memory/atomic.h>

#include <fstream>

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;

using boost::static_pointer_cast;
using boost::dynamic_pointer_cast;
using boost::tuple;
using boost::make_shared;
using boost::shared_ptr;
using boost::shared_array;

using eflib::fixed_string;

using namespace salviar;
using namespace sasl::codegen;
using namespace sasl::common;
using namespace sasl::syntax_tree;
using namespace sasl::host;
using namespace sasl::semantic;
using namespace sasl::shims;

BEGIN_NS_SASL_HOST();

host_impl::host_impl()
{
	sasl_create_ia_shim(ia_shim_);
	sasl_create_interp_shim(interp_shim_);

	ia_shim_func_			= nullptr;
	vx_shader_func_			= nullptr;

	sa_						= nullptr;
	input_layout_			= nullptr;

	px_shader_				= nullptr;
	vx_shader_				= nullptr;

	vso2reg_func_			= nullptr;
	interp_func_			= nullptr;
	reg2psi_func_			= nullptr;
	vx_shader_func_			= nullptr;
	stream_descs_			= nullptr;
}

void host_impl::initialize(render_stages const* stages)
{
	sa_ = stages->assembler.get();
}

void host_impl::update(render_state const* state)
{
	// TODO: Need to reduce shim generates by detecting state changes.
	input_layout_	= state->layout.get();
	vx_shader_		= state->vx_shader.get();
	// px_shader_		= state->px_shader.get();

	if (!sa_ || !input_layout_ || !vx_shader_)
	{
		ia_shim_func_	= nullptr;
		vx_shader_func_ = nullptr;
		stream_descs_	= nullptr;
		vso2reg_func_	= nullptr;
		interp_func_	= nullptr;
		reg2psi_func_	= nullptr;
		return;
	}

	// Compute shim function.
	void* ia_shim_func_typeless = ia_shim_->get_shim_function(
		ia_shim_slots_, ia_shim_element_offsets_, ia_shim_dest_offsets_,
		input_layout_, vx_shader_->get_reflection()
		);
	ia_shim_func_	= reinterpret_cast<ia_shim_func_ptr>(ia_shim_func_typeless);
	vx_shader_func_ = vx_shader_->native_function<shader_func_ptr>();

	// Compute stream descs for input assembler.
	if( !ia_shim_slots_.empty() )
	{
		stream_descs_ = &(sa_->get_stream_descs(ia_shim_slots_)[0]);
	}

	// Update shims
	interp_shim_->get_shim_functions(
		&vso2reg_func_,
		&interp_func_,
		&reg2psi_func_,
		interp_modifiers_,
		vso_offsets_,
		vso_types_,
		psi_offsets_,
		psi_types_,
		vx_shader_ ? vx_shader_->get_reflection() : NULL,
		px_shader_ ? px_shader_->get_reflection() : NULL
		);

	// Update vertex buffers from state.
	vx_cbuffer_.resize( vx_shader_->get_reflection()->total_size(su_buffer_in) );
	
	for(auto const& variable: state->vx_cbuffer.variables())
	{
		auto const& var_name = variable.first;
		auto const& var_data = variable.second;
		auto var_data_addr = state->vx_cbuffer.data_pointer(var_data);

		sv_layout* layout = vx_shader_->get_reflection()->input_sv_layout(var_name);

        if(!layout) continue;

		if(layout->agg_type == aggt_array)
		{
			vx_update_constant_pointer(var_name, var_data_addr);
		}
		else
		{
			vx_update_constant(var_name, var_data_addr, var_data.length);
		}
	}

	for(auto const& samp: state->vx_cbuffer.samplers())
	{
		vx_update_sampler(samp.first, samp.second);
	}
}

void host_impl::update_target_params(renderer_parameters const& /*rp*/, buffer_ptr const& /*target*/)
{
}

vx_shader_unit_ptr host_impl::get_vx_shader_unit() const
{
	shader_reflection const* vx_reflection = vx_shader_->get_reflection();

	size_t attrs_count = vx_reflection->layouts_count(su_buffer_out);
	
	ia_shim_data data;
	data.stream_descs	= stream_descs_;
	data.dest_offsets	= &(ia_shim_dest_offsets_[0]);
	data.element_offsets= &(ia_shim_element_offsets_[0]);
	data.count			= ia_shim_slots_.size();

	vx_shader_unit_impl* ret = new vx_shader_unit_impl(
		ia_shim_func_,
		vx_shader_func_,
		&(vx_cbuffer_[0]),
		&data,
		vx_reflection->total_size(salviar::su_stream_in),
		vx_reflection->total_size(salviar::su_buffer_out),
		vx_reflection->total_size(salviar::su_stream_out),
		vso2reg_func_,
		static_cast<uint32_t>(attrs_count),
		vso_offsets_.empty() ? NULL : &(vso_offsets_[0]),
		vso_types_.empty()   ? NULL : &(vso_types_[0])
		);
	return vx_shader_unit_ptr(ret);
}

px_shader_unit_ptr host_impl::get_px_shader_unit() const
{
	return px_shader_unit_ptr();
}

size_t host_impl::vs_output_attr_count() const
{
	return vx_shader_ ? vx_shader_->get_reflection()->layouts_count(su_buffer_out) - 1 : 0;
}

bool host_impl::vx_update_constant(fixed_string const& name, void const* value, size_t sz)
{
	if(!vx_shader_) return false;
	sv_layout* layout = vx_shader_->get_reflection()->input_sv_layout(name);
	if(!layout || layout->size != sz) return false;
	memcpy(&(vx_cbuffer_[layout->offset]), value, layout->size);
	return true;
}

bool host_impl::vx_update_constant_pointer(fixed_string const& name, void const* pvalue)
{
	void const* array_ptr[2] = {pvalue, pvalue};
	return vx_update_constant(name, &(array_ptr[0]), sizeof(void const*));
}

bool host_impl::vx_update_sampler(fixed_string const& name, sampler_ptr const& samp)
{
	sampler* psamp = samp.get();
	return vx_update_constant(name, &psamp, sizeof(void*));
}

END_NS_SASL_HOST();

using namespace sasl::host;
using namespace salviar;

void salvia_create_host(host_ptr& out)
{
	out.reset( new host_impl() );
}

void salvia_compile_shader_impl(
	shader_object_ptr& out_shader_object,
	shader_log_ptr& out_logs,
	std::string const& code_or_file_name,
	shader_profile const& profile,
	vector<external_function_desc> const& external_funcs,
	bool from_file
	)
{
	out_shader_object.reset();

	boost::shared_ptr<sasl::drivers::compiler> drv;
	sasl_create_compiler(drv);

	if(from_file)
	{
		drv->set_code_file(code_or_file_name);
	}
	else
	{
		drv->set_code(code_or_file_name);
	}

	const char* lang_name = NULL;
	switch(profile.language)
	{
	case lang_pixel_shader:
		lang_name = "--lang=ps";
		break;
	case lang_vertex_shader:
		lang_name = "--lang=vs";
		break;
	default:
		lang_name = "--lang=g";
		break;
	}

	drv->set_parameter(lang_name);
	shared_ptr<diag_chat> results = drv->compile(external_funcs);

	shader_log_impl_ptr log_impl = make_shared<shader_log_impl>();
	out_logs = log_impl;
	for(size_t i = 0; i < results->diag_items().size(); ++i)
	{
		log_impl->append(
			sasl::common::str(results->diag_items()[i])
			);
	}

	shader_object_impl_ptr ret;
	if( error_count(results.get(), false) == 0 )
	{
		ret.reset( new shader_object_impl() );
		ret->set_reflection	( drv->get_reflection() );
		ret->set_vm_code	( drv->get_vmcode() );
	}
	out_shader_object = ret;

	return;
}

void salvia_compile_shader(
	shader_object_ptr& out_shader_object,
	shader_log_ptr& out_logs,
	std::string  const& code,
	shader_profile const& profile,
	vector<external_function_desc> const& external_funcs
	)
{
	salvia_compile_shader_impl(
		out_shader_object, out_logs,
		code, profile, external_funcs,
		false
		);
}

void salvia_compile_shader_file(
	shader_object_ptr& out_shader_object,
	shader_log_ptr& out_logs,
	std::string  const& file_name,
	shader_profile const& profile,
	vector<external_function_desc> const& external_funcs
	)
{
	salvia_compile_shader_impl(
		out_shader_object, out_logs,
		file_name, profile, external_funcs,
		true
		);
}
