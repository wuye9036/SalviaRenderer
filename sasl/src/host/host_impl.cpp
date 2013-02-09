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

	ia_shim_func_			= NULL;
	vx_shader_func_			= NULL;
	stream_descs_			= NULL;
	sa_						= NULL;
}

void host_impl::initialize(stream_assembler* sa)
{
	sa_ = sa;
}

void host_impl::buffers_changed()
{
	update_stream_descs();
}

void host_impl::update_stream_descs()
{
	if( ia_shim_slots_.empty() )
	{
		stream_descs_ = NULL;
	}
	else
	{
		stream_descs_ = &(sa_->get_stream_descs(ia_shim_slots_)[0]);
	}
}

void host_impl::update_ia_shim_func()
{
	// Update shim function.
	if (!sa_ || !sa_->layout() || !vx_shader_)
		return;

	void* shim_func = ia_shim_->get_shim_function(
		ia_shim_slots_, ia_shim_element_offsets_, ia_shim_dest_offsets_,
		sa_->layout(), vx_shader_->get_reflection()
		);
	update_stream_descs();
	ia_shim_func_ = static_cast<ia_shim_func_ptr>(shim_func);
}

void host_impl::update_interp_funcs()
{
	assert(interp_shim_);

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
}

void host_impl::input_layout_changed()
{
	update_ia_shim_func();
}

void host_impl::update_vertex_shader(shader_object_ptr const& vso)
{
	// Update shim and shader native function.
	vx_shader_ = vso;
	update_ia_shim_func();

	void* shader_func = vx_shader_->native_function();
	vx_shader_func_ = static_cast<shader_func_ptr>(shader_func);

	update_interp_funcs();

	// Reset all cached constant and samplers.
	vx_cbuffer_.resize( vx_shader_->get_reflection()->total_size(su_buffer_in) );
	vx_dynamic_cbuffers_.clear();
	sampler_cache_.clear();
}

void host_impl::update_pixel_shader(shader_object_ptr const& /*pso*/)
{
}

void host_impl::update_target_params(render_parameters const& /*rp*/, buffer_ptr const& /*target*/)
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

bool host_impl::vx_set_constant(fixed_string const& name, void const* value)
{
	if(!vx_shader_) return false;
	sv_layout* layout = vx_shader_->get_reflection()->input_sv_layout(name);
	if(!layout) return false;
	memcpy(&(vx_cbuffer_[layout->offset]), value, layout->size);
	return true;
}

bool host_impl::vx_set_constant_pointer(fixed_string const& name, void const* pvalue, size_t sz)
{
	shared_array<char> data_array(new char[sz]);
	memcpy(data_array.get(), pvalue, sz);
	vx_dynamic_cbuffers_[name] = data_array;

	void* array_ptr[2] = {data_array.get(), data_array.get()};
	return vx_set_constant( name, &(array_ptr[0]) );
}

bool host_impl::vx_set_sampler(fixed_string const& name, sampler_ptr const& samp)
{
	sampler_cache_.push_back(samp);
	sampler* psamp = samp.get();
	return vx_set_constant(name, &psamp);
}

END_NS_SASL_HOST();

using namespace sasl::host;
using namespace salviar;

void salvia_create_host(host_ptr& out)
{
	out.reset( new host_impl() );
}

void salvia_compile_shader(
	shader_object_ptr& out_shader_object,
	shader_log_ptr& out_logs,
	std::string  const& code,
	shader_profile const& profile,
	vector<external_function_desc> const& external_funcs
	)
{
	out_shader_object.reset();
	
	boost::shared_ptr<sasl::drivers::compiler> drv;
	sasl_create_compiler(drv);
	drv->set_code(code);

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

	shader_object_impl_ptr ret( new shader_object_impl() );

	ret->set_reflection	( drv->get_reflection() );
	ret->set_vm_code	( drv->get_vmcode() );

	out_shader_object = ret;
	return;
}
