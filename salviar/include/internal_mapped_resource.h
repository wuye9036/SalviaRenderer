#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

namespace salviar{

struct internal_mapped_resource
{
	internal_mapped_resource(boost::function<void* (size_t)> realloc)
		: reallocator(realloc)
	{
	}
	void*		data;
	uint32_t	row_pitch;
	uint32_t	depth_pitch;

	boost::function<void* (size_t)>
			reallocator;
};

}
