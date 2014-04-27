#pragma once

namespace eflib
{
	template <typename T>
	intptr_t ptr_to_addr(T* p)
	{
		return reinterpret_cast<intptr_t>(p);
	}
	
	template <typename T>
	T* addr_to_ptr(intptr_t a)
	{
		return reinterpret_cast<T*>(a);
	}
	
	template <typename T>
	T* advance_bytes(T* p, size_t dist)
	{
		return addr_to_ptr<T>(ptr_to_addr(p) + dist);
	}

	template <typename T>
	intptr_t distance_bytes(T const* a, T const* b)
	{
		return ptr_to_addr(b) - ptr_to_addr(a);
	}
}