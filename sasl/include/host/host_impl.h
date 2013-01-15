#ifndef SASL_HOST_HOST_IMPL_H
#define SASL_HOST_HOST_IMPL_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/host.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace salviar
{
	class  stream_assembler;
	struct stream_desc;
	struct external_function_desc;

	EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
	EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
}

namespace sasl
{
	namespace shims
	{
		struct shim_data;
		EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
		EFLIB_DECLARE_CLASS_SHARED_PTR(interp_shim);
		EFLIB_DECLARE_CLASS_SHARED_PTR(om_shim);
	}
}

BEGIN_NS_SASL_HOST();

EFLIB_DECLARE_CLASS_SHARED_PTR(host_impl);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log_impl);

typedef void (*ia_shim_func_ptr)(
	void*					output_buffer,
	shims::shim_data const* data,
	size_t					i_vert
	);

typedef void (*shader_func_ptr)(
	void const* input_stream,  void const* input_buffer,
	void*       output_stream, void*       output_buffer
	);

class host_impl: public salviar::host
{
public:
	host_impl();

	void initialize(salviar::stream_assembler* sa);

	void buffers_changed		();
	void input_layout_changed	();

	void update_vertex_shader	(salviar::shader_object_ptr const& vso);
	void update_pixel_shader	(salviar::shader_object_ptr const& pso);
	void update_target_params	(salviar::render_parameters const& rp, 
								salviar::buffer_ptr const& target);

	bool vx_set_constant		(eflib::fixed_string const&, void const* value);
	bool vx_set_constant_pointer(eflib::fixed_string const&, void const* pvalue, size_t sz);
	bool vx_set_sampler			(eflib::fixed_string const&, salviar::sampler_ptr const& samp);

	salviar::vx_shader_unit_ptr get_vx_shader_unit() const;
	salviar::px_shader_unit_ptr get_px_shader_unit() const;
	
private:
	// Data used by Shim and Shader
	salviar::stream_assembler*	sa_;

	shims::om_shim_ptr			om_shim_;
	shims::interp_shim_ptr		interp_shim_;
	shims::ia_shim_ptr			ia_shim_;
	
	salviar::shader_object_ptr	px_shader_;
	salviar::shader_object_ptr	vx_shader_;

	// Cached data
	std::vector<char>			vx_cbuffer_;
	std::vector<char>			px_cbuffer_;
	boost::unordered_map<
		eflib::fixed_string,
		boost::shared_array<char> 
	>							vx_dynamic_cbuffers_;
	std::vector<salviar::sampler_ptr>
								sampler_cache_;

	ia_shim_func_ptr			ia_shim_func_;
	std::vector<size_t>			ia_shim_slots_;
	size_t*						ia_shim_dest_offsets_;

	shader_func_ptr				vx_shader_func_;
	salviar::stream_desc const*	stream_descs_;

};

END_NS_SASL_HOST();

extern "C"
{
	SASL_HOST_API void salvia_create_host   (salviar::host_ptr& out);
	SASL_HOST_API void salvia_compile_shader(
		salviar::shader_object_ptr& out_shader_object,
		salviar::shader_log_ptr& out_logs,
		std::string const& code,
		salviar::shader_profile const& profile,
		std::vector<salviar::external_function_desc> const& external_funcs
		);
}

#endif