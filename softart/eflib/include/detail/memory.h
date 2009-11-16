#ifndef EFLIB_MEMORY_H
#define EFLIB_MEMORY_H

#include <boost/type_traits.hpp>
#include <limits>

#include <stdlib.h>

#ifdef max
#	undef max
#	undef min
#endif

namespace efl{

//∂‘∆Î∫Í
#ifdef _MSC_VER
#	define ALIGN16	__declspec(align(16))
#	define ALIGN4	__declspec(align(4))
#else
#	define ALIGN16
#	define ALIGN4
#endif


	/***************
	 * ∂‘∆Î∑÷≈‰£¨From KlayGE
	 ***************/
	template <typename T, int alignment>
	class aligned_allocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		static const int alignment_size = alignment;

		BOOST_STATIC_ASSERT(0 == (alignment & (alignment - 1)));
		BOOST_STATIC_ASSERT(alignment <= 65536);

		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, alignment> other;
		};

		pointer address(reference val) const
		{
			return &val;
		}

		const_pointer address(const_reference val) const
		{
			return &val;
		}

		aligned_allocator() throw()
		{
		}

		aligned_allocator(const aligned_allocator<T, alignment>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator(const aligned_allocator<U, alignment2>&) throw()
		{
		}

		~aligned_allocator() throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator<T, alignment>& operator=(const aligned_allocator<U, alignment2>&)
		{
			return *this;
		}

		void deallocate(pointer p, size_type)
		{
			uint16_t* p16 = reinterpret_cast<uint16_t*>(p);
			uint8_t* org_p = reinterpret_cast<uint8_t*>(p16) - p16[-1];
			free(org_p);
		}

		pointer allocate(size_type count)
		{
			uint8_t* p = static_cast<uint8_t*>(malloc(count * sizeof (T) + 2 + (alignment - 1)));
			uint8_t* new_p = reinterpret_cast<uint8_t*>((reinterpret_cast<size_t>(p) + 2 + (alignment - 1)) & (-static_cast<int32_t>(alignment)));
			reinterpret_cast<uint16_t*>(new_p)[-1] = static_cast<uint16_t>(new_p - p);

			return reinterpret_cast<pointer>(new_p);
		}

		pointer allocate(size_type count, const void* hint)
		{
			pointer* p = this->allocate(count);
			memcpy(p, hint, count * sizeof(T));
			this->deallocate(hint);
			return p;
		}

		void construct(pointer p, const T& val)
		{
			void* vp = p;
			::new (vp) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}

		size_type max_size() const throw()
		{
			return std::numeric_limits<size_t>::max() / sizeof(T);
		}
	};

	template <typename T, int alignment1, typename U, int alignment2>
	inline bool operator==(const aligned_allocator<T, alignment1>&, const aligned_allocator<U, alignment2>&) throw()
	{
		return true;
	}

	template <typename T, int alignment1, typename U, int alignment2>
	inline bool operator!=(const aligned_allocator<T, alignment1>&, const aligned_allocator<U, alignment2>&) throw()
	{
		return false;
	}


	template <int alignment>
	class aligned_allocator<void, alignment>
	{
	public:
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef void value_type;

		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, alignment> other;
		};

		aligned_allocator() throw()
		{
		}

		aligned_allocator(const aligned_allocator<void, alignment>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator(const aligned_allocator<U, alignment2>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator<void, alignment>& operator=(const aligned_allocator<U, alignment2>&)
		{
			return *this;
		}
	};

	template <int aligned_size>
	struct is_aligned
	{
		template <class T>
		bool check(T p)
		{
			BOOST_STATIC_ASSERT(boost::is_pointer<T>::value);

			return ((size_t)p % aligned_size == 0);
		}
	};

#define  is_aligned(pointer, aligned_size) (efl::is_aligned<aligned_size>().check(pointer))
}

#endif
