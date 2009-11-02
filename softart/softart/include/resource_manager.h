#ifndef SOFTART_RESOURCE_MANAGER_H
#define SOFTART_RESOURCE_MANAGER_H

#include "buffer.h"
#include "texture.h"

class buffer_manager
{
public:
	h_buffer create_buffer(size_t size){
		return boost::shared_ptr<buffer>(new buffer(size));
	}

	void release_buffer(h_buffer& hbuf){
		if(hbuf){
			hbuf.reset();
			return;
		}
		custom_assert(false, "");
	}
};

class texture_manager
{
public:
	h_texture create_texture_2d(size_t width, size_t height, pixel_format fmt){
		return h_texture(new texture_2d(width, height, fmt));
	}
	h_texture create_texture_cube(size_t width, size_t height, pixel_format fmt){
		return h_texture(new texture_cube(width, height, fmt));
	}

	void release_texture(h_texture& htex){
		if(htex){
			htex.reset();
			return;
		}
		custom_assert(false, "");
	}
};

#endif