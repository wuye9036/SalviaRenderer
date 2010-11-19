#include "../include/stream_assembler.h"

#include "../include/shaderregs_op.h"
#include "../include/buffer.h"
#include "../include/shader.h"

BEGIN_NS_SOFTART();

using namespace eflib;

vec4 get_vec4(input_type type, input_register_usage_decl usage, const void* data)
{
	bool is_valid_type = 
		(0 <= type && type < input_type_count);
	bool is_valid_usage = 
		(0 <= usage && usage < input_register_usage_decl_count);
	bool is_valid_data = (data != NULL);

	EFLIB_ASSERT(is_valid_type, "");
	EFLIB_ASSERT(is_valid_usage, "");
	EFLIB_ASSERT(is_valid_data, "");

	if( ! (is_valid_type && is_valid_usage && is_valid_data) ){
		return vec4::zero();
	}

	const float* floats = (const float*)data;

	float w_comp = 1.0f;
	if(usage == input_register_usage_attribute){
		w_comp = 0.0f;
	}
		
	switch(type){
		case input_float:
			return vec4(floats[0], 0.0f, 0.0f, 1.0f);
		case input_float2:
			return vec4(floats[0], floats[1], 0.0f, 1.0f);
		case input_float3:
			return vec4(floats[0], floats[1], floats[2], 1.0f);
	}

	return vec4(floats[0], floats[1], floats[2], floats[3]);
}

void stream_assembler::set_input_layout(const input_layout_decl& layout)
{
	layout_ = layout;
}

void stream_assembler::fetch_vertex(vs_input& rv, size_t idx)
{
	for(size_t i_elemdecl = 0; i_elemdecl < layout_.size(); ++i_elemdecl){
		const input_element_decl& ied = layout_[i_elemdecl];
		size_t sidx = ied.stream_idx;
		size_t ridx = ied.regidx;
		
		EFLIB_ASSERT(sidx < streams_.size(), "");
		EFLIB_ASSERT(ridx < vsi_attrib_regcnt, "");
		
		if(sidx >= streams_.size() || ridx >= vsi_attrib_regcnt){
			return;
		}
		
		const h_buffer& hb = streams_[sidx];
		EFLIB_ASSERT(hb, "");
		if(!hb) return;

		const uint8_t* pdata = hb->raw_data(ied.offset + ied.stride * idx);
		rv.attributes[sidx] = get_vec4(ied.type, ied.usage, pdata);
	}
}

void stream_assembler::set_stream(stream_index stridx, h_buffer hbuf)
{
	EFLIB_ASSERT(0 <= stridx && stridx < stream_index_count, "");
	
	if(size_t(stridx) >= streams_.size()){
		streams_.resize(stridx+1);
	}

	streams_[stridx] = hbuf;
}

size_t stream_assembler::num_vertices() const
{
	const input_element_decl& ied = layout_[0];
	h_buffer hb = streams_[ied.stream_idx];
	EFLIB_ASSERT(hb, "");

	return hb->get_size() / ied.stride;
}

END_NS_SOFTART()
