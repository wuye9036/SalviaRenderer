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

using boost::make_tuple;
using boost::get;
using boost::tuple;

using std::vector;
using std::pair;
using std::make_pair;

BEGIN_NS_SALVIAR();

vec4 get_vec4(format fmt, semantic_value const& sv, const void* data)
{
	assert( data );

	if( !data ){ return vec4::zero(); }

	const float* floats = (const float*)data;

	float w_comp = ( sv.get_system_value() == sv_position ) ? 1.0f : 0.0f;
		
	switch(fmt){
		case format_r32_float:
			return vec4(floats[0], 0.0f, 0.0f, w_comp);
		case format_r32g32_float:
			return vec4(floats[0], floats[1], 0.0f, w_comp);
		case format_r32g32b32_float:
			return vec4(floats[0], floats[1], floats[2], w_comp);
		case format_r32g32b32a32_float:
			return vec4(floats[0], floats[1], floats[2], floats[3]);
		case format_r32g32b32a32_sint:
			return *reinterpret_cast<vec4 const*>(floats);
		default:
			assert(false);
	}

	return vec4::zero();
}

void stream_assembler::set_input_layout(input_layout_ptr const& layout)
{
	layout_ = layout;
}

/// Only used by Cpp Vertex Shader
void stream_assembler::update_register_map( boost::unordered_map<semantic_value, size_t> const& reg_map ){
	reg_and_pelem_and_buffer_index.clear();
	reg_and_pelem_and_buffer_index.reserve( reg_map.size() );

	typedef pair<semantic_value, size_t> pair_t;
	BOOST_FOREACH( pair_t const& sv_reg_pair, reg_map ){
		input_element_desc const* elem_desc = layout_.lock()->find_desc( sv_reg_pair.first );
		int buf_index = find_buffer( elem_desc->input_slot );
		
		if( buf_index == -1 || elem_desc == NULL ){
			reg_and_pelem_and_buffer_index.clear();
			return;
		}

		reg_and_pelem_and_buffer_index.push_back(
			make_tuple( sv_reg_pair.second, elem_desc, static_cast<size_t>(buf_index) ) 
			);
	}
}

/// Only used by Cpp Vertex Shader
void stream_assembler::fetch_vertex(vs_input& rv, size_t vert_index) const
{
	typedef tuple<size_t, input_element_desc const*, size_t> tuple_t;
	BOOST_FOREACH(
		tuple_t const& reg_elem_buffer_index,
		reg_and_pelem_and_buffer_index )
	{
		size_t reg_index = get<0>( reg_elem_buffer_index );
		input_element_desc const * desc = get<1>( reg_elem_buffer_index );
		size_t buffer_index = get<2>( reg_elem_buffer_index );

		void const* pdata = element_address( buffer_index, desc->aligned_byte_offset, vert_index );
		rv.attribute(reg_index) = get_vec4( desc->data_format, semantic_value(desc->semantic_name, desc->semantic_index), pdata);
	}
}

void const* stream_assembler::element_address( input_element_desc const& elem_desc, size_t vert_index ) const{
	int buffer_index = find_buffer( elem_desc.input_slot );
	if( buffer_index == -1 ){ return NULL; }
	return element_address( buffer_index, elem_desc.aligned_byte_offset, vert_index );
}

void const* stream_assembler::element_address( semantic_value const& sv, size_t vert_index ) const{
	return element_address(*( layout_.lock()->find_desc(sv) ), vert_index);
}

void const* stream_assembler::element_address( size_t buffer_index, size_t member_offset, size_t vert_index ) const{
	return vbufs_[buffer_index]->raw_data( member_offset + strides_[buffer_index] * vert_index + offsets_[buffer_index] );
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
		size_t index = find_buffer(slot);

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

input_layout_ptr stream_assembler::layout() const
{
	return layout_.lock();
}

int stream_assembler::find_buffer(size_t slot) const
{
	vector<size_t>::const_iterator slot_it = find( slots_.begin(), slots_.end(), slot );
	if( slot_it == slots_.end() ){
		return -1;
	}
	return static_cast<int>( distance( slots_.begin(), slot_it ) );
}

vector<stream_desc> const& stream_assembler::get_stream_descs(vector<size_t> const& slots)
{
	stream_descs_.clear();

	for(size_t i_slot = 0; i_slot < slots.size(); ++i_slot)
	{
		size_t slot = slots[i_slot];
		stream_desc str_desc;
		int buffer_index	= find_buffer(slot);

		if(buffer_index == -1)
		{
			str_desc.buffer = NULL;
			str_desc.offset = 0;
			str_desc.stride = 0;
		}
		else
		{
			str_desc.buffer = vbufs_[buffer_index]->raw_data(0);
			str_desc.offset = offsets_[buffer_index];
			str_desc.stride = strides_[buffer_index];
		}
		
		stream_descs_.push_back(str_desc);
	}

	return stream_descs_;
}

END_NS_SALVIAR();
