#include <salviar/include/buffer.h>
#include <salviar/include/index_fetcher.h>

BEGIN_NS_SALVIAR();

void index_fetcher::initialize(h_buffer hbuf, format index_fmt, primitive_topology primtopo, uint32_t startpos, uint32_t basevert)
{
	indexbuf_ = hbuf;
	index_fmt_ = index_fmt;
	primtopo_ = primtopo;
	if (format_r16_uint == index_fmt_)
	{
		stride_ = 2;
	}
	else
	{
		EFLIB_ASSERT(format_r32_uint == index_fmt_, "Index type is wrong.");
		stride_ = 4;
	}
	startpos_ = startpos * stride_;
	basevert_ = basevert;
}

void index_fetcher::fetch_indices(uint32_t* prim_indices, uint32_t id)
{
	//TODO: need support feature "index restart" from DX 10 Spec

	uint32_t count;
	uint32_t ids[3];
	switch(primtopo_)
	{
	case primitive_line_list:
		count = 2;
		ids[0] = id * 2 + 0;
		ids[1] = id * 2 + 1;
		break;

	case primitive_line_strip:
		count = 2;
		ids[0] = id + 0;
		ids[1] = id + 1;
		break;
	
	case primitive_triangle_list:
		count = 3;
		ids[0] = id * 3 + 0;
		ids[1] = id * 3 + 1;
		ids[2] = id * 3 + 2;
		break;

	case primitive_triangle_strip:
		count = 3;
		ids[0] = id + 0;
		ids[1] = id + 1;
		ids[2] = id + 2;

		if(id & 1){
			std::swap(ids[0], ids[2]);
		}
		break;

	default:
		EFLIB_ASSERT_UNEXPECTED();
		count = 0;
		break;
	}

	if (indexbuf_)
	{
		if (format_r16_uint == index_fmt_)
		{
			uint16_t* pidx = reinterpret_cast<uint16_t*>(indexbuf_->raw_data(startpos_));
			for (uint32_t i = 0; i < count; ++ i)
			{
				prim_indices[i] = pidx[ids[i]] + basevert_;
			}
		}
		else
		{
			EFLIB_ASSERT(format_r32_uint == index_fmt_, "Index type is wrong.");

			uint32_t* pidx = reinterpret_cast<uint32_t*>(indexbuf_->raw_data(startpos_));
			for (uint32_t i = 0; i < count; ++ i)
			{
				prim_indices[i] = pidx[ids[i]] + basevert_;
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < count; ++ i)
		{
			prim_indices[i] = ids[i] + basevert_;
		}
	}
}

END_NS_SALVIAR();
