#include <salviar/include/resource_data.h>

#include <eflib/include/math/math.h>

BEGIN_NS_SALVIAR();

aligned_array::aligned_array(size_t sz, size_t alignment)
	: size_(sz)
{
	assert(alignment & (alignment-1) != 0);
	alignment_ = eflib::ceil_to_pow2( static_cast<int>(alignment) );
	reallocate();
}

void aligned_array::reallocate()
{
	size_t aligned_size = size_ + alignment_ - 1;

	data_.reset( new char[aligned_size] );
	uintptr_t addr = reinterpret_cast<uintptr_t>( data_.get() );
	if( addr % alignment_ != 0 )
	{
		addr = addr + alignment_ - (addr % alignment_);
	}
	aligned_data_ = reinterpret_cast<char*>(addr);
}

void* aligned_array::data() const
{
	return aligned_data_;
}

size_t aligned_array::size() const
{
	return size_;
}

size_t aligned_array::alignment() const
{
	return alignment_;
}

END_NS_SALVIAR();