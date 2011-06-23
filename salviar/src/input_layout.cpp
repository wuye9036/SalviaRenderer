#include <salviar/include/input_layout.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

BEGIN_NS_SALVIAR();

void input_layout::slot_range( size_t& min_slot, size_t& max_slot ) const{
	min_slot = std::numeric_limits<size_t>::max();
	max_slot = 0;

	BOOST_FOREACH( input_element_desc const& elem_desc, descs ){
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
	return NULL;
}

input_element_desc const* input_layout::find_desc( semantic_value const& v ) const{
	for( size_t i_desc = 0; i_desc < descs.size(); ++i_desc ){
		if( semantic_value( descs[i_desc].semantic_name,  descs[i_desc].semantic_index ) == v ){
			return &( descs[i_desc] );
		}
	}
	return NULL;
}

END_NS_SALVIAR();