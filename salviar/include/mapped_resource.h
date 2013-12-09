#pragma once

BEGIN_NS_SALVIAR();

struct mapped_resource
{
	void*		data;
	uint32_t	row_pitch;
	uint32_t	depth_pitch;
};

END_NS_SALVIAR();