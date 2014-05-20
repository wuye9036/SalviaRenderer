#include <salviar/include/buffer.h>
#include <salviar/include/render_state.h>
#include <salviar/include/index_fetcher.h>

BEGIN_NS_SALVIAR();

void index_fetcher::update(render_state const* state)
{
    index_buffer_	= state->cmd == command_id::draw_index ? state->index_buffer.get() : nullptr;
	index_format_	= state->index_format;
	prim_topo_		= state->prim_topo;

	if (format_r16_uint == index_format_)
	{
		stride_ = 2;
	}
	else
	{
		EFLIB_ASSERT(format_r32_uint == index_format_, "Index type is wrong.");
		stride_ = 4;
	}
	start_addr_ = state->start_index * stride_;
	base_vert_  = state->base_vertex;
}

void index_fetcher::fetch_indexes(uint32_t* indexes_of_prim, uint32_t* min_index, uint32_t* max_index, uint32_t prim_id_beg, uint32_t prim_id_end)
{
	uint32_t* out_indexes = indexes_of_prim;
	uint32_t  prim_vert_count = 0;
	uint32_t  min_id = 0;
	uint32_t  max_id = 0;
	uint32_t const min_prim_id = prim_id_beg;
	uint32_t const max_prim_id = prim_id_end - 1;

	switch(prim_topo_)
	{
	case primitive_line_list:
		prim_vert_count = 2;
		min_id = min_prim_id * 2;
		max_id = max_prim_id * 2 + 1;
		break;

	case primitive_line_strip:
		prim_vert_count = 2;
		min_id = min_prim_id;
		max_id = max_prim_id + 1;
		break;
	
	case primitive_triangle_list:
		prim_vert_count = 3;
		min_id = min_prim_id * 3;
		max_id = max_prim_id * 3 + 2;
		break;

	case primitive_triangle_strip:
		prim_vert_count = 3;
		min_id = min_prim_id;
		max_id = max_prim_id + 2;
		break;

	default:
		EFLIB_ASSERT_UNEXPECTED();
		return;
	}

	if(index_buffer_)
	{
		*min_index = std::numeric_limits<uint32_t>::max();
		*max_index = 0;
	}

	for(uint32_t prim_id = prim_id_beg; prim_id < prim_id_end; ++prim_id)
	{
		uint32_t ids[3];

		switch(prim_topo_)
		{
		case primitive_line_list:
			ids[0] = prim_id * 2 + 0;
			ids[1] = prim_id * 2 + 1;
			break;

		case primitive_line_strip:
			ids[0] = prim_id + 0;
			ids[1] = prim_id + 1;
			break;
	
		case primitive_triangle_list:
			ids[0] = prim_id * 3 + 0;
			ids[1] = prim_id * 3 + 1;
			ids[2] = prim_id * 3 + 2;
			break;

		case primitive_triangle_strip:
			ids[0] = prim_id + 0;
			ids[1] = prim_id + 1;
			ids[2] = prim_id + 2;

			if(prim_id & 1)
			{
				std::swap(ids[0], ids[2]);
			}
			break;

		default:
			break;
		}

		if (index_buffer_)
		{
			//TODO: need support feature "index restart" from DX 10 Spec
			uint8_t* index_buffer_address = index_buffer_->raw_data(start_addr_);
			if (format_r16_uint == index_format_)
			{
				uint16_t* pidx = reinterpret_cast<uint16_t*>(index_buffer_address);
				for (uint32_t i = 0; i < prim_vert_count; ++i)
				{
					uint16_t biased_index = pidx[ids[i]];
					*min_index = std::min<uint32_t>(biased_index, *min_index);
					*max_index = std::max<uint32_t>(biased_index, *max_index);
					out_indexes[i] = biased_index + base_vert_;
				}
			}
			else
			{
				EFLIB_ASSERT(format_r32_uint == index_format_, "Index type is wrong.");

				uint32_t* pidx = reinterpret_cast<uint32_t*>(index_buffer_address);
				for (uint32_t i = 0; i < prim_vert_count; ++ i)
				{
					uint32_t biased_index = pidx[ids[i]];
					*min_index = std::min(biased_index, *min_index);
					*max_index = std::max(biased_index, *max_index);
					out_indexes[i] = biased_index + base_vert_;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < prim_vert_count; ++ i)
			{
				out_indexes[i] = ids[i] + base_vert_;
			}
		}

		out_indexes += prim_vert_count;
	}

	*min_index += base_vert_;
	*max_index += base_vert_;
}

END_NS_SALVIAR();
