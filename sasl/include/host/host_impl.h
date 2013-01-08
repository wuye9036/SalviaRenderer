#ifndef SASL_HOST_HOST_IMPL_H
#define SASL_HOST_HOST_IMPL_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/host.h>
#include <salviar/include/shader_object.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_HOST();

EFLIB_DECLARE_CLASS_SHARED_PTR(host_impl);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log_impl);

class host_impl: public salviar::host
{
public:
	salviar::shader_object_ptr compile(

		);
		
	void set_input_layout	(salviar::input_layout_ptr const& il);
	void set_vertex_shader	(salviar::shader_object_ptr const& vso);
	void set_pixel_shader	(salviar::shader_object_ptr const& pso);
	void set_target_params	(salviar::render_parameters const& rp, 
							 salviar::buffer_ptr const& target);
	
	salviar::vx_shader_unit_ptr get_vx_shader_unit() const;
	salviar::px_shader_unit_ptr get_px_shader_unit() const;
	
private:
	void*						vx_cbuffer_;
	void*						px_cbuffer_;
	
	//om_shim_ptr				om_shim_;
	//interp_shim_ptr			interp_shim_;
	//ia_shim_ptr				ia_shim_;
	
	salviar::shader_object_ptr	px_shader_;
	salviar::shader_object_ptr	vx_shader_;
};

END_NS_SASL_HOST();

extern "C"
{
	SASL_HOST_API salviar::host* salvia_create_host();
	SASL_HOST_API void			 salvia_compile_shader(
		salviar::shader_object_ptr& out_shader_object,
		salviar::shader_log_ptr& out_logs,
		salviar::shader_profile const&,
		std::string const& code
		);
};

#endif