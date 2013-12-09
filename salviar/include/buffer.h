#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/memory/allocator.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>
#include <memory.h>

BEGIN_NS_SALVIAR();

struct internal_mapped_resource;

class buffer
{
	std::vector< uint8_t,
		eflib::aligned_allocator<uint8_t, 16>
	>		data_;

public:
	buffer(size_t size): data_(size){}

	size_t			size() const{ return data_.size(); }
	uint8_t const*	raw_data(size_t offset) const { return data_.data() + offset; }
	uint8_t*		raw_data(size_t offset) { return data_.data() + offset;}

	result map(internal_mapped_resource& mapped, map_mode mm);
	result unmap(internal_mapped_resource& mapped, map_mode mm);

	void transfer(size_t offset, void const* psrcdata, size_t size, size_t count);
	void transfer(size_t offset, void const* psrcdata, size_t stride_dest, size_t stride_src, size_t size, size_t count);
};

END_NS_SALVIAR();
