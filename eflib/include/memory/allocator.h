#ifndef EFLIB_MEMORY_ALLOCATOR_H
#define EFLIB_MEMORY_ALLOCATOR_H

#include <boost/type_traits.hpp>
#include <limits>
#include <vector>
#include <stdlib.h>

#ifdef max
#	undef max
#	undef min
#endif

namespace eflib{

#ifdef _MSC_VER
#	define ALIGN16	__declspec(align(16))
#	define ALIGN4	__declspec(align(4))
#else
#	define ALIGN16
#	define ALIGN4
#endif


	/**
	 * Aligned allocator£¬from KlayGE
	 */
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
			pointer p = this->allocate(count);
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

#define  is_aligned(pointer, aligned_size) (eflib::is_aligned<aligned_size>().check(pointer))

	template <typename T>
	class unified_allocator{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		template <typename U>
		struct rebind{
			typedef unified_allocator<U> other;
		};

		pointer address(reference val) const{
			return &val;
		}

		const_pointer address(const_reference val) const{
			return &val;
		}

		unified_allocator() throw()	{}

		unified_allocator(const unified_allocator<T>&) throw()
		{
		}

		template <typename U>
		unified_allocator(const unified_allocator<U>&) throw() {}

		~unified_allocator() throw() {}

		template <typename U>
		unified_allocator<T>& operator=(const unified_allocator<U>&){
			return *this;
		}

		void deallocate(pointer p, size_type)
		{
			uintptr_t* new_p = reinterpret_cast<uintptr_t*>(p);
			void (*pfree)( void* ) = reinterpret_cast< void(*)(void*) >( new_p[-1] );
			pfree( new_p-1 );
		}

		pointer allocate(size_type count)
		{
			uintptr_t* p = reinterpret_cast<uintptr_t*>( malloc( count * sizeof(T) + sizeof(uintptr_t) ) );
			p[0] = reinterpret_cast<uintptr_t>( static_cast<void (*)(void*)>( &free ) );
			return reinterpret_cast<pointer>( p + 1 );
		}

		pointer allocate(size_type count, const void* hint)
		{
			pointer p = this->allocate(count);
			memcpy(p, hint, count * sizeof(T));
			this->deallocate(hint);
			return p;
		}

		void construct(pointer p, const T& val)
		{
			void* vp = p;
			::new (vp) T(val);
		}

		void destroy(pointer p){
			p->~T();
		}

		size_type max_size() const throw(){
			return std::numeric_limits<size_t>::max() / sizeof(T);
		}
	};

	template <typename T, typename U>
	inline bool operator==(const unified_allocator<T>&, const unified_allocator<U>&) throw()
	{
		return true;
	}

	template <typename T, typename U>
	inline bool operator!=(const unified_allocator<T>&, const unified_allocator<U>&) throw()
	{
		return false;
	}

	template<>
	class unified_allocator<void>
	{
	public:
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef void value_type;

		template <typename U>
		struct rebind
		{
			typedef unified_allocator<U> other;
		};

		unified_allocator() throw()
		{
		}

		unified_allocator(const unified_allocator<void>&) throw()
		{
		}

		template <typename U, int>
		unified_allocator(const unified_allocator<U>&) throw()
		{
		}

		template <typename U>
		unified_allocator<void>& operator=(const unified_allocator<U>&)
		{
			return *this;
		}
	};
}

#endif
