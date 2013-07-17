#include <salviar/include/stream_assembler.h>

#include <salviar/include/input_layout.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/buffer.h>
#include <salviar/include/shader.h>
#include <salviar/include/render_state.h>
#include <salviar/include/stream_state.h>

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
		
	switch(fmt)
	{
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

void stream_assembler::update(render_state const* state)
{
	layout_				= state->layout.get();
	stream_buffer_descs_= state->str_state->buffer_descs.data();
}

/// Only used by Cpp Vertex Shader
void stream_assembler::update_register_map( boost::unordered_map<semantic_value, size_t> const& reg_map )
{
	register_to_input_element_desc.clear();
	register_to_input_element_desc.reserve( reg_map.size() );

	typedef pair<semantic_value, size_t> pair_t;
	for(auto const& sv_reg_pair: reg_map)
	{
		input_element_desc const* elem_desc = layout_->find_desc(sv_reg_pair.first);
		
		if(elem_desc == nullptr)
		{
			register_to_input_element_desc.clear();
			return;
		}

		register_to_input_element_desc.push_back(
			make_pair(sv_reg_pair.second, elem_desc) 
			);
	}
}

/// Only used by Cpp Vertex Shader
void stream_assembler::fetch_vertex(vs_input& rv, size_t vert_index) const
{
	typedef tuple<size_t, input_element_desc const*, size_t> tuple_t;
	for(auto const& reg_ied_pair: register_to_input_element_desc )
	{
		auto reg_index	= reg_ied_pair.first;
		auto desc		= reg_ied_pair.second;

		void const* pdata = element_address(*desc, vert_index);
		rv.attribute(reg_index) = get_vec4( desc->data_format, semantic_value(desc->semantic_name, desc->semantic_index), pdata);
	}
}

void const* stream_assembler::element_address( input_element_desc const& elem_desc, size_t vert_index ) const
{
	auto buf_desc = stream_buffer_descs_ + elem_desc.input_slot;
	return buf_desc->buf->raw_data( elem_desc.aligned_byte_offset + buf_desc->stride * vert_index + buf_desc->offset );
}

void const* stream_assembler::element_address( semantic_value const& sv, size_t vert_index ) const{
	return element_address(*( layout_->find_desc(sv) ), vert_index);
}

vector<stream_desc> const& stream_assembler::get_stream_descs(vector<size_t> const& slots)
{
	stream_descs_.resize( slots.size() );

	for(size_t i_slot = 0; i_slot < slots.size(); ++i_slot)
	{
		size_t slot = slots[i_slot];
		stream_desc& str_desc = stream_descs_[i_slot];

		str_desc.buffer = stream_buffer_descs_[slot].buf->raw_data(0);
		str_desc.offset = stream_buffer_descs_[slot].offset;
		str_desc.stride = stream_buffer_descs_[slot].stride;
	}

	return stream_descs_;
}

END_NS_SALVIAR();
