#pragma once

#ifndef SASL_SHIMS_IA_SHIM_H
#define SASL_SHIMS_IA_SHIM_H

#include <sasl/include/shims/shims_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_array.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace LLVM
{
	class Module;
	class IRBuilder;
}

namespace salviar
{
	struct stream_desc;
	EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
	EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection);
}

BEGIN_NS_SASL_SHIMS();

struct ia_shim_key
{
	ia_shim_key(
		salviar::input_layout* input,
		salviar::shader_reflection const* reflection)
		: input(input), reflection(reflection) {}

	salviar::input_layout*		input;
	salviar::shader_reflection const*	reflection;

	bool operator == (ia_shim_key const& rhs) const
	{
		return input == rhs.input && reflection == rhs.reflection;
	}
};

size_t hash_value(ia_shim_key const&);

struct ia_shim_data
{
	salviar::stream_desc const*	stream_descs;
	intptr_t const*				element_offsets;// TODO: OPTIMIZED BY JIT
	size_t const*				dest_offsets;	// TODO: OPTIMIZED BY JIT
	size_t						count;			// TODO: OPTIMIZED BY JIT
};

EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
class ia_shim
{
public:
	static ia_shim_ptr create();

	virtual void* get_shim_function(
		std::vector<size_t>&				used_slots,
		std::vector<intptr_t>&				aligned_element_offsets,
		std::vector<size_t>&				dest_offsets,
		salviar::input_layout*				input,
		salviar::shader_reflection const*	reflection
	);
private:
	struct shim_func_data
	{
		void*					func;
		std::vector<size_t>		dest_offsets;
		std::vector<size_t>		used_slots;
		std::vector<intptr_t>	aligned_element_offsets;
	};
	typedef boost::unordered_map<ia_shim_key, shim_func_data> cached_shim_function_dict;
	cached_shim_function_dict cached_shim_funcs_;
};

END_NS_SASL_SHIMS();

#endif

