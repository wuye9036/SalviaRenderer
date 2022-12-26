#pragma once

#include <salviar/include/salviar_forward.h>

#include <salvia/core/render_capacity.h>

#include <eflib/utility/shared_declaration.h>

#include <array>

namespace salviar{

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);

struct stream_buffer_desc
{
	stream_buffer_desc()
		: slot(0), stride(0), offset(0)
	{
	}

	size_t		slot;
	buffer_ptr	buf;
	size_t		stride;
	size_t		offset;
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(stream_state);

struct stream_state
{
	std::array<stream_buffer_desc, MAX_INPUT_SLOTS> buffer_descs;
	
	void update(
		size_t starts_slot,
		size_t buffers_count, buffer_ptr const* bufs,
		size_t const* strides, size_t const* offsets
		);
};

}
