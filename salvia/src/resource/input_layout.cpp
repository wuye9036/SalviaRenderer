#include <salviar/include/input_layout.h>

#include <eflib/platform/boost_begin.h>
#include <boost/range/iterator_range.hpp>
#include <eflib/platform/boost_end.h>

#include <algorithm>
#include <memory>

using std::make_shared;

namespace salviar {

void input_layout::slot_range( size_t& min_slot, size_t& max_slot ) const{
	min_slot = std::numeric_limits<size_t>::max();
	max_slot = 0;

	for(input_element_desc const& elem_desc: descs )
	{
		min_slot = std::min<size_t>( min_slot, elem_desc.input_slot );
		max_slot = std::max<size_t>( max_slot, elem_desc.input_slot );
	}

	if( max_slot < min_slot ){
		max_slot = min_slot = 0;
	}
}

input_layout::iterator input_layout::desc_begin() const{
	return descs.begin();
}

input_layout::iterator input_layout::desc_end() const{
	return descs.end();
}

semantic_value input_layout::get_semantic( iterator it ) const{
	return semantic_value( it->semantic_name, it->semantic_index );
}

size_t input_layout::find_slot( semantic_value const& v ) const{
	input_element_desc const* elem_desc = find_desc( v );
	if( elem_desc ){
		return elem_desc->input_slot;
	}
	return std::numeric_limits<size_t>::max();
}

input_element_desc const* input_layout::find_desc( size_t slot ) const{
	for( size_t i_desc = 0; i_desc < descs.size(); ++i_desc ){
		if( slot == descs[i_desc].input_slot ){
			return &( descs[i_desc] );
		}
	}
	return nullptr;
}

input_element_desc const* input_layout::find_desc( semantic_value const& v ) const{
	for( size_t i_desc = 0; i_desc < descs.size(); ++i_desc ){
		if( semantic_value( descs[i_desc].semantic_name,  descs[i_desc].semantic_index ) == v ){
			return &( descs[i_desc] );
		}
	}
	return nullptr;
}

input_layout_ptr input_layout::create( input_element_desc const* pdesc, size_t desc_count, shader_object_ptr const& /*vs*/ ){
	input_layout_ptr ret = make_shared<input_layout>();
	ret->descs.assign( pdesc, pdesc + desc_count );

	// Check shader code.
	// Caculate member offset.

	return ret;
}

input_layout_ptr input_layout::create( input_element_desc const* pdesc, size_t desc_count, cpp_vertex_shader_ptr const& /*vs*/ ){
	input_layout_ptr ret = make_shared<input_layout>();
	ret->descs.assign( pdesc, pdesc + desc_count );

	// Check vertex shader.
	// Caculate member offset.

	return ret;
}

size_t hash_value(input_element_desc const& v)
{
	size_t seed = 0;

	eflib::hash_combine(seed, v.aligned_byte_offset);
	eflib::hash_combine(seed, static_cast<size_t>(v.data_format) );
	eflib::hash_combine(seed, v.input_slot);
	eflib::hash_combine(seed, v.instance_data_step_rate);
	eflib::hash_combine(seed, v.semantic_index);
	eflib::hash_combine(seed, v.semantic_name);
	eflib::hash_combine(seed, static_cast<size_t>(v.slot_class) );

	return seed;
}

size_t hash_value(input_layout const& v)
{
	return eflib::hash_range( v.desc_begin(), v.desc_end() );
}

END_NS_SALVIAR()