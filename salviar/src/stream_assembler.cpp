#include <salviar/include/stream_assembler.h>

#include <salviar/include/input_layout.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/buffer.h>
#include <salviar/include/shader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace eflib;
using boost::make_iterator_range;
using std::vector;

BEGIN_NS_SALVIAR();

vec4 get_vec4(input_formats fmt, semantic_value const& sv, const void* data)
{
	assert( 0 <= fmt && fmt < input_formats_count );
	assert( data );

	if( !data ){ return vec4::zero(); }

	const float* floats = (const float*)data;

	float w_comp = ( sv.get_system_value() == sv_position ) ? 1.0f : 0.0f;
		
	switch(fmt){
		case input_float:
			return vec4(floats[0], 0.0f, 0.0f, w_comp);
		case input_float2:
			return vec4(floats[0], floats[1], 0.0f, w_comp);
		case input_float3:
			return vec4(floats[0], floats[1], floats[2], w_comp);
		default:
			return vec4::zero();
	}

	return vec4(floats[0], floats[1], floats[2], floats[3]);
}

void stream_assembler::set_input_layout( input_layout const* layout ){
	layout_ = layout;
}

void stream_assembler::fetch_vertex(vs_input& rv, size_t vert_index) const
{
	BOOST_FOREACH(
		input_element_desc const& elem_desc,
		make_iterator_range( layout_->desc_begin(), layout_->desc_end() ) 
		)
	{
		// TODO Correct stream mapping need to be implemented.
		size_t slot = elem_desc.input_slot;
		if( slot >= vsi_attrib_regcnt ){
			return;
		}

		void const* pdata = element_address( elem_desc, vert_index );
		if( !pdata ){ return; }

		// TODO Matched with semantic but not slot.
		rv.attributes[slot] = get_vec4( elem_desc.format, semantic_value(elem_desc.semantic_name, elem_desc.semantic_index), pdata);
	}
}

void const* stream_assembler::element_address( input_element_desc const& elem_desc, size_t index ) const{
	int buf_index = buffer_index( elem_desc.input_slot );
	if( buf_index == -1 ){ return NULL; }
	return vbufs_[buf_index]->raw_data( elem_desc.aligned_byte_offset + strides_[buf_index] * index + offsets_[buf_index] );
}

void const* stream_assembler::element_address( semantic_value const& sv, size_t index ) const{
	return element_address( layout_->semantic_desc( sv ), index );
}

void stream_assembler::set_vertex_buffers(
	size_t starts_slot,
	size_t buffers_count, h_buffer const* pbufs,
	size_t const* strides, size_t const* offsets
	)
{
	if( !pbufs || !strides || !offsets ) { return; }

	for( size_t i_buf = 0; i_buf < buffers_count; ++i_buf ){
		size_t slot = starts_slot + i_buf;
		size_t index = buffer_index(slot);

		if( index == -1 ){
			strides_.push_back( strides[i_buf] );
			offsets_.push_back( offsets[i_buf] );
			slots_.push_back(slot);
			vbufs_.push_back(pbufs[i_buf]);
		} else {
			strides_[index] = strides[i_buf];
			offsets_[index] = offsets[i_buf];
			slots_[index] = slot;
			vbufs_[index] = pbufs[i_buf];
		}
	}
}

size_t stream_assembler::num_vertices() const{
	if( vbufs_.empty() ){ return 0; }
	return ( vbufs_[0]->get_size() - offsets_[0] ) / strides_[0];
}

input_layout const* stream_assembler::layout() const{
	return layout_;
}

int stream_assembler::buffer_index( size_t slot ) const{
	vector<size_t>::const_iterator slot_it = find( slots_.begin(), slots_.end(), slot );
	if( slot_it == slots_.end() ){
		return -1;
	}
	return distance( slots_.begin(), slot_it );
}

END_NS_SALVIAR();
