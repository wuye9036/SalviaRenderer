#include <salviar/include/stream_assembler.h>

#include <salviar/include/input_layout.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/buffer.h>
#include <salviar/include/shader.h>

using std::vector;
using namespace eflib;

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

void stream_assembler::fetch_vertex(vs_input& rv, size_t idx)
{
	for(size_t i_elem = 0; i_elem < layout_->desc_size(); ++i_elem){
		
		// TODO Correct stream mapping need to be implemented.

		size_t max_slot = *( std::max_element( slots_.begin(), slots_.end() ) );

		input_element_desc const& elem_desc = layout_->get_desc(i_elem);
		size_t slot = elem_desc.input_slot;

		if( slot > max_slot || slot >= vsi_attrib_regcnt ){
			return;
		}

		vector<size_t>::iterator slot_it = find( slots_.begin(), slots_.end(), slot );
		if( slot_it == slots_.end() ){
			return;
		}
		
		int buf_index = distance( slot_it, slots_.begin() );
		h_buffer const& buf = vbufs_[ buf_index ];
		if( !buf ) { return; }

		uint8_t const* pdata = buf->raw_data( strides_[buf_index] * idx + offsets_[buf_index] + elem_desc.aligned_byte_offset );

		// TODO Matched with semantic.
		rv.attributes[i_elem] = get_vec4( elem_desc.format, semantic_value(elem_desc.semantic_name, elem_desc.semantic_index), pdata);
	}
}

void stream_assembler::set_vertex_buffers(
	size_t starts_slot,
	size_t buffers_count, h_buffer* pbufs,
	size_t const* strides, size_t const* offsets
	)
{
	if( !pbufs || !strides || !offsets ) { return; }

	for( size_t i_buf = 0; i_buf < buffers_count; ++i_buf ){
		size_t slot = starts_slot + i_buf;
		vector<size_t>::iterator slot_it = find( slots_.begin(), slots_.end(), slot );

		if( slot_it == slots_.end() ){
			strides_.push_back( strides[i_buf] );
			offsets_.push_back( offsets[i_buf] );
			slots_.push_back(slot);
			vbufs_.push_back(pbufs[i_buf]);
		} else {
			int index = distance( slots_.begin(), slot_it );
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

vector<h_buffer> const& stream_assembler::streams() const{
	return vbufs_;
}

END_NS_SALVIAR()
