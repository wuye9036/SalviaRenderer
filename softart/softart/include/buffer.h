#ifndef SOFTART_BUFFER_H
#define SOFTART_BUFFER_H

#include "eflib/include/eflib.h"
#include "enums.h"

#include <boost/shared_ptr.hpp>

#include <vector>

#include "softart_fwd.h"
BEGIN_NS_SOFTART()

class buffer
{
	std::vector<uint8_t> bufdata_;
	bool is_locked_;
	bool islocked();

public:
	buffer(size_t size):bufdata_(size){}

	size_t get_size() const{ return bufdata_.size(); }
	const uint8_t* raw_data(size_t offset) const {return &(bufdata_[offset]);}
	uint8_t* raw_data(size_t offset) {return &(bufdata_[offset]);}

	void lock(void** pdata, lock_mode lm, size_t offset, size_t size);
	template <class T>
	void lock(T** pdata, lock_mode lm, size_t offset, size_t size);
	
	void unlock();
	void transfer(size_t offset, void* psrcdata, size_t stride_dest, size_t stride_src, size_t size, size_t count)
	{
		custom_assert(offset + stride_dest * (count - 1) + size <= get_size(), "");
		if( offset + stride_dest * (count - 1) + size > get_size() ) return;

		byte* dest = raw_data(offset);
		byte* src = (byte*)psrcdata;
		for(size_t i = 0; i < count; ++i)
		{
			memcpy(dest, src, size);
			src += stride_src;
			dest += stride_dest;
		}
	}
};

END_NS_SOFTART()

#endif