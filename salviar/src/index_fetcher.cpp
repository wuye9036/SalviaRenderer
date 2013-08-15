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

void index_fetcher::fetch_indexes(uint32_t* indexes_of_prim, uint32_t prim_id_beg, uint32_t prim_id_end)
{
	//TODO: need support feature "index restart" from DX 10 Spec
	uint8_t* index_buffer_address = index_buffer_->raw_data(start_addr_);
	
	uint32_t* out_indexes = indexes_of_prim;

	for(uint32_t prim_id = prim_id_beg; prim_id < prim_id_end; ++prim_id)
	{
		uint32_t count;
		uint32_t ids[3];

		switch(prim_topo_)
		{
		case primitive_line_list:
			count = 2;
			ids[0] = prim_id * 2 + 0;
			ids[1] = prim_id * 2 + 1;
			break;

		case primitive_line_strip:
			count = 2;
			ids[0] = prim_id + 0;
			ids[1] = prim_id + 1;
			break;
	
		case primitive_triangle_list:
			count = 3;
			ids[0] = prim_id * 3 + 0;
			ids[1] = prim_id * 3 + 1;
			ids[2] = prim_id * 3 + 2;
			break;

		case primitive_triangle_strip:
			count = 3;
			ids[0] = prim_id + 0;
			ids[1] = prim_id + 1;
			ids[2] = prim_id + 2;

			if(prim_id & 1)
			{
				std::swap(ids[0], ids[2]);
			}
			break;

		default:
			EFLIB_ASSERT_UNEXPECTED();
			count = 0;
			break;
		}

		if (index_buffer_)
		{
			if (format_r16_uint == index_format_)
			{
				uint16_t* pidx = reinterpret_cast<uint16_t*>(index_buffer_address);
				for (uint32_t i = 0; i < count; ++ i)
				{
					out_indexes[i] = pidx[ids[i]] + base_vert_;
				}
			}
			else
			{
				EFLIB_ASSERT(format_r32_uint == index_format_, "Index type is wrong.");

				uint32_t* pidx = reinterpret_cast<uint32_t*>(index_buffer_address);
				for (uint32_t i = 0; i < count; ++ i)
				{
					out_indexes[i] = pidx[ids[i]] + base_vert_;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < count; ++ i)
			{
				out_indexes[i] = ids[i] + base_vert_;
			}
		}

		out_indexes += count;
	}
}

END_NS_SALVIAR();
