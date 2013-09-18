#pragma once

#include <salviar/include/enums.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/memory/allocator.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

class buffer
{
	std::vector< uint8_t, 
		eflib::aligned_allocator<uint8_t, 16>
	>		bufdata_;
	bool	is_locked_;

	bool islocked();

public:
	buffer(size_t size):bufdata_(size), is_locked_(false){}

	size_t get_size() const{ return bufdata_.size(); }
	const uint8_t* raw_data(size_t offset) const {return &(bufdata_[offset]);}
	uint8_t* raw_data(size_t offset) {return &(bufdata_[offset]);}

	void map(void** pdata, map_mode mm, size_t offset, size_t size);
	template <class T>
	void map(T** pdata, map_mode mm, size_t offset, size_t size);
	void unmap();

	void transfer( size_t offset, void const* psrcdata, size_t size, size_t count ){
		EFLIB_ASSERT_AND_IF(offset + size * count <= get_size(), "Out of boundary of buffer.")
		{
			return;
		}

		byte* dest = raw_data(offset);
		byte* src = (byte*)psrcdata;

		memcpy( dest, src, size*count );
	}

	void transfer(size_t offset, void const* psrcdata, size_t stride_dest, size_t stride_src, size_t size, size_t count)
	{
		EFLIB_ASSERT_AND_IF(offset + stride_dest * (count - 1) + size <= get_size(), "Out of buffer."){
			return;
		}

		byte* dest = raw_data(offset);
		byte* src = (byte*)psrcdata;

		if( stride_dest == stride_src && stride_src == size ){
			// Optimized for continuous memory layout.
			transfer( offset, psrcdata, size, count );
		} else {
			for(size_t i = 0; i < count; ++i){
				memcpy(dest, src, size);
				src += stride_src;
				dest += stride_dest;
			}
		}
	}
};

END_NS_SALVIAR();
