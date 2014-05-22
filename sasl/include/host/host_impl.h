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

namespace eflib
{
	template <typename T, int Size> struct vector_;
	typedef vector_<float, 4> vec4;
}

namespace salviar
{
	class  stream_assembler;
	struct stream_desc;
	struct external_function_desc;
	struct render_state;
	struct render_stages;
	class  input_layout;

	EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
	EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
}

namespace sasl
{
	namespace shims
	{
		struct ia_shim_data;
		EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
		EFLIB_DECLARE_CLASS_SHARED_PTR(interp_shim);
		EFLIB_DECLARE_CLASS_SHARED_PTR(om_shim);
	}
}

BEGIN_NS_SASL_HOST();

EFLIB_DECLARE_CLASS_SHARED_PTR(host_impl);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log_impl);

typedef void (*ia_shim_func_ptr)(
	void*						output_buffer,
	shims::ia_shim_data const*	data,
	size_t						i_vert
	);

typedef void (*shader_func_ptr)(
	void const* input_stream,  void const* input_buffer,
	void*       output_stream, void*       output_buffer
	);

typedef void (*vso2reg_func_ptr)(
	eflib::vec4*		out_registers,
	void const*			in_data,
	intptr_t const*		in_offsets,			// TODO: OPTIMIZED BY JIT
	uint32_t const*		in_value_types,		// TODO: OPTIMIZED BY JIT
	uint32_t			register_count		// TODO: OPTIMIZED BY JIT
	);

typedef void (*interp_func_ptr)(
	eflib::vec4*		out_registers,
	eflib::vec4 const*	in_registers,
	uint32_t const*		interp_modifiers,	// TODO: OPTIMIZED BY JIT
	uint32_t			register_count		// TODO: OPTIMIZED BY JIT
	);

typedef void (*reg2psi_func_ptr)(
	void const*			out_data,
	intptr_t*			out_offsets,		// TODO: OPTIMIZED BY JIT
	uint32_t*			out_value_types,	// TODO: OPTIMIZED BY JIT
	eflib::vec4 const*	in_registers,
	uint32_t			register_count		// TODO: OPTIMIZED BY JIT
	);

class host_impl: public salviar::host
{
public:
	host_impl();

	void initialize				(salviar::render_stages const* stages);

	void update					(salviar::render_state const* state);
	void update_target_params	(salviar::renderer_parameters const& rp, 
								salviar::buffer_ptr const& target);

	salviar::vx_shader_unit_ptr get_vx_shader_unit() const;
	salviar::px_shader_unit_ptr get_px_shader_unit() const;

	size_t						vs_output_attr_count() const;
	
private:
	// Data used by Shim and Shader
	salviar::stream_assembler*	sa_;
	salviar::input_layout*		input_layout_;

	shims::om_shim_ptr			om_shim_;
	shims::interp_shim_ptr		interp_shim_;
	shims::ia_shim_ptr			ia_shim_;
	
	salviar::shader_object*		px_shader_;
	salviar::shader_object*		vx_shader_;

	// Cached data
	std::vector<char>			vx_cbuffer_;
	std::vector<char>			px_cbuffer_;

	ia_shim_func_ptr			ia_shim_func_;
	std::vector<size_t>			ia_shim_slots_;
	std::vector<intptr_t>		ia_shim_element_offsets_;
	std::vector<size_t>			ia_shim_dest_offsets_;
	
	vso2reg_func_ptr			vso2reg_func_;
	interp_func_ptr				interp_func_;
	reg2psi_func_ptr			reg2psi_func_;
	std::vector<intptr_t>		vso_offsets_;
	std::vector<uint32_t>		vso_types_;
	std::vector<intptr_t>		psi_offsets_;
	std::vector<uint32_t>		psi_types_;
	std::vector<uint32_t>		interp_modifiers_;

	shader_func_ptr				vx_shader_func_;
	salviar::stream_desc const*	stream_descs_;

	bool vx_update_constant			(eflib::fixed_string const&, void const* value,	size_t sz);
	bool vx_update_constant_pointer	(eflib::fixed_string const&, void const* value);
	bool vx_update_sampler			(eflib::fixed_string const&, salviar::sampler_ptr const& samp);

	void update_stream_descs();
	void update_ia_shim_func();
	void update_interp_funcs();
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
	SASL_HOST_API void salvia_compile_shader_file(
		salviar::shader_object_ptr& out_shader_object,
		salviar::shader_log_ptr& out_logs,
		std::string const& code,
		salviar::shader_profile const& profile,
		std::vector<salviar::external_function_desc> const& external_funcs
		);
}

#endif