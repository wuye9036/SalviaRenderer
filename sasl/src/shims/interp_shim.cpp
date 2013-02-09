#include <sasl/include/shims/interp_shim.h>

#include <salviar/include/shader_reflection.h>
#include <salviar/include/shader_impl.h>

using namespace salviar;

using eflib::vec4;
using std::vector;

BEGIN_NS_SASL_SHIMS();

void default_vso2reg(
	vec4*			out_registers,
	void const*		in_data,
	intptr_t const*	in_offsets,			// TODO: OPTIMIZED BY JIT
	uint32_t const*	in_value_types,		// TODO: OPTIMIZED BY JIT
	uint32_t		register_count		// TODO: OPTIMIZED BY JIT
	)
{
	for(size_t i_register = 0; i_register < register_count; ++i_register)
	{
		void const* attr_data
			= static_cast<char const*>(in_data) + in_offsets[i_register];
		vec4* current_register = out_registers + i_register;
		language_value_types lvt
			= static_cast<language_value_types>(in_value_types[i_register]);
		switch(lvt)
		{
		case salviar::lvt_f32v4:
			memcpy(current_register, attr_data, sizeof(float)*4);
			continue;
		case salviar::lvt_f32v3:
			memcpy(current_register, attr_data, sizeof(float)*3);
			continue;
		default:
			assert(!"unsupported lvt");
		}
	}
}

interp_shim_ptr interp_shim::create()
{
	return interp_shim_ptr( new interp_shim() );
}

void interp_shim::get_shim_functions(
	vso2reg_func_ptr*			out_vso2reg_fn,
	interp_func_ptr*			out_interp_fn,
	reg2psi_func_ptr*			out_reg2psi_fn,
	vector<uint32_t>&			out_interp_modifiers,
	vector<intptr_t>&			out_vso_offsets,
	vector<uint32_t>&			out_vso_types,
	vector<intptr_t>&			out_psi_offsets,
	vector<uint32_t>&			out_psi_types,
	shader_reflection const*	vs_reflection,
	shader_reflection const*	ps_reflection )
{
	*out_vso2reg_fn = NULL;
	*out_interp_fn = NULL;
	*out_reg2psi_fn = NULL;
	out_interp_modifiers.clear();
	out_vso_offsets.clear();
	out_vso_types.clear();
	out_psi_offsets.clear();
	out_psi_types.clear();

	if(vs_reflection)
	{
		if(ps_reflection)
		{
			assert(false);
		}
		else
		{
			vector<sv_layout*> vs_layouts = vs_reflection->layouts(su_buffer_out);

			out_interp_modifiers.resize( vs_layouts.size() );
			out_vso_offsets		.resize( vs_layouts.size() );
			out_vso_types		.resize( vs_layouts.size() );

			size_t output_index = 1;
			for(size_t i_layout = 0; i_layout < vs_layouts.size(); ++i_layout)
			{
				sv_layout* layout = vs_layouts[i_layout];
				if( layout->sv == semantic_value(sv_position) )
				{
					out_interp_modifiers[0] = im_linear | im_noperspective;
					out_vso_offsets[0]		= layout->offset;
					out_vso_types[0]		= layout->value_type;
				}
				else
				{
					out_interp_modifiers[output_index]	= im_linear;
					out_vso_offsets[output_index]		= layout->offset;
					out_vso_types[output_index]			= layout->value_type;
					++output_index;
				}
			}

			*out_vso2reg_fn = &default_vso2reg;
		}
	}
	else
	{
		if(ps_reflection)
		{
			assert(false);
		}
		else
		{
			return;
		}
	}
}


END_NS_SASL_SHIMS();