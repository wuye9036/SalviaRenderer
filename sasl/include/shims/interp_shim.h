#pragma once

#ifndef SASL_SHIMS_INTERP_SHIM_H
#define SASL_SHIMS_INTERP_SHIM_H

#include <sasl/include/shims/shims_forward.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace eflib
{
	template <typename T, int Size> struct vector_;
	typedef vector_<float, 4> vec4;
}

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection);
}

BEGIN_NS_SASL_SHIMS();

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

EFLIB_DECLARE_CLASS_SHARED_PTR(interp_shim);

class interp_shim
{
public:
	static interp_shim_ptr create();
	
	/**
	get_shim_functions
		out_vso2reg_fn:
			Generated function will converts vs output to interpolation registers.
		out_interp_fn:
			Interpolation function.
		out_reg2psi_fn:
			Generated function will converts interpolation registers to ps input.
		out_vso_offsets:
			Offsets of all fields of vs output.
			It could be optimized, see section 'remark' for details.
		out_vso_types:
			Types of all fields of vs output.
			It could be optimized, see section 'remark' for details.
		out_psi_offsets:
			Offsets of all fields of ps input.
			It could be optimized, see section 'remark' for details.
		out_psi_types:
			Types of all fields of ps input.
			It could be optimized, see section 'remark' for details.
		vs_reflection:
			vertex shader reflection.
		ps_reflection:
			pixel shader reflection.

	Remark:
		If ps_reflection is available, vso_offsets and vso_types will be optimized for ps input.
		for e.g.,
		if vso and psi are 

			struct vso { float2 Pos; int2 tex0; int4 tex1; }
			struct psi { float2 Pos; float2 tex1; }
		
		vso_offsets may includes 2 items, vso::Pos and vso::tex1,
		since psi will use these two only.

		Similar with above, psi_offsets and psi_types will be optimized
		if vs_reflection is available and not all fields in ps input
		were provided by vertex shader output.
	*/
	void get_shim_functions(
		vso2reg_func_ptr*					out_vso2reg_fn,
		interp_func_ptr*					out_interp_fn,
		reg2psi_func_ptr*					out_reg2psi_fn,
		std::vector<uint32_t>&				out_interp_modifiers,
		std::vector<intptr_t>&				out_vso_offsets,
		std::vector<uint32_t>&				out_vso_types,
		std::vector<intptr_t>&				out_psi_offsets,
		std::vector<uint32_t>&				out_psi_types,
		salviar::shader_reflection const*	vs_reflection,
		salviar::shader_reflection const*	ps_reflection
		);
};

END_NS_SASL_SHIMS();

#endif