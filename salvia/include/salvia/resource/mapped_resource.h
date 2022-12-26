#pragma once

namespace salviar{

struct mapped_resource
{
	void*		data;
	uint32_t	row_pitch;
	uint32_t	depth_pitch;
};

}