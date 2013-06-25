#pragma once

#include <salviar/include/buffer.h>
#include <salviar/include/texture.h>
#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

class resource_manager
{
public:
	buffer_ptr create_buffer(size_t size)
	{
		return boost::shared_ptr<buffer>(new buffer(size));
	}

	texture_ptr create_texture_2d(size_t width, size_t height, size_t num_samples, pixel_format fmt)
	{
		return texture_ptr(new texture_2d(width, height, num_samples, fmt));
	}
	
	texture_ptr create_texture_cube(size_t width, size_t height, size_t num_samples, pixel_format fmt)
	{
		return texture_ptr(new texture_cube(width, height, num_samples, fmt));
	}
};

END_NS_SALVIAR();
