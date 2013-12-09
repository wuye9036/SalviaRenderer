#pragma once

#include <salviar/include/salviar_forward.h>
#include <salviar/include/enums.h>
#include <salviar/include/buffer.h>
#include <salviar/include/texture.h>
#include <salviar/include/internal_mapped_resource.h>

#include <eflib/include/memory/allocator.h>

#include <vector>

BEGIN_NS_SALVIAR();

struct mapped_resource;

class resource_manager
{
public:
	resource_manager(boost::function<void ()> sync)
		: renderer_sync_(sync)
		, map_mode_ (map_mode_none)
		, mapped_resource_([this](size_t sz)-> void* { return this->reallocate_buffer(sz); })
	{
	}

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

	result map(mapped_resource&, buffer_ptr const& buf, map_mode mm);
	result map(mapped_resource&, surface_ptr const& surf, map_mode mm);

	result unmap();

private:
	template <typename T> result map_impl(mapped_resource&, T const& res, map_mode mm);
	void* reallocate_buffer(size_t sz);

	boost::function<void ()>
		renderer_sync_;		// Synchronize/Flush with renderer command queue.
	std::vector<uint8_t, eflib::aligned_allocator<uint8_t, 16>>
		mapped_data_;		// Temporary mapped data.
	internal_mapped_resource
		mapped_resource_;
	map_mode
		map_mode_;
	surface_ptr
		mapped_surf_;
	buffer_ptr
		mapped_buf_;
};

END_NS_SALVIAR();
