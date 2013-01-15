#include <sasl/include/shims/ia_shim.h>

#include <salviar/include/input_layout.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_reflection.h>
#include <salviar/include/stream_assembler.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/functional/hash.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <utility>

using namespace salviar;
using std::vector;
using std::make_pair;

BEGIN_NS_SASL_SHIMS();

size_t hash_value(ia_shim_key const& key)
{
	size_t seed = 0;

	boost::hash_combine(seed, key.input);		// Input layout we need to hash all fields.
	boost::hash_combine(seed, key.reflection);	// Shader layout we just use address as hash code.
	
	return seed;
}

ia_shim_ptr ia_shim::create()
{
	return ia_shim_ptr( new ia_shim() );
}

void common_ia_shim(void* output_buffer, shim_data const* mapping, size_t ivert);

void* ia_shim::get_shim_function(
		std::vector<size_t>&				used_slots,
		size_t**							dest_offsets,
		salviar::input_layout_ptr  const&	input,
		salviar::shader_reflection const*	reflection
	)
{
	ia_shim_key key(input, reflection);
	cached_shim_function_dict::iterator iter = cached_shim_funcs_.find(key);

	if (iter != cached_shim_funcs_.end() )
	{
		used_slots		= iter->second.used_slots;
		*dest_offsets	= iter->second.dest_offsets.get();
		return iter->second.func;
	}
	
	// Compute output address and input slots.
	vector<sv_layout*> layouts = reflection->layouts(su_stream_in);
	
	shim_func_data data;

	data.func = &common_ia_shim;
	data.dest_offsets.reset( new size_t[layouts.size()] );
	*dest_offsets = reinterpret_cast<size_t*>( data.dest_offsets.get() );

	for(size_t i_layout = 0; i_layout < layouts.size(); ++i_layout)
	{
		sv_layout* layout = layouts[i_layout];
		input_element_desc const* element_desc = input->find_desc(layout->sv);
		data.used_slots.push_back(element_desc->input_slot);
		(*dest_offsets)[i_layout] = layout->offset;
	}

	cached_shim_funcs_.insert( make_pair(key, data) );

	used_slots	   = data.used_slots;
	return &common_ia_shim;
}

void common_ia_shim(void* output_buffer, shim_data const* mapping, size_t ivert)
{
	char* output_start = static_cast<char*>(output_buffer);

	for(size_t i = 0; i < mapping->count; ++i)
	{
		stream_desc const& str_desc	= mapping->stream_descs[i];
		char*		 source_start	= static_cast<char*>(str_desc.buffer);
		char*		 source_ptr		= source_start + str_desc.offset +str_desc.stride * ivert;
		char*		 output_addr	= output_start + mapping->dest_offsets[i];
		*reinterpret_cast<void**>(output_addr) = source_ptr;
	}
}

END_NS_SASL_SHIMS();