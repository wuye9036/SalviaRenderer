#pragma once

#include <eflib/include/memory/allocator.h>

#include <utility>
#include <iterator>

namespace eflib 
{
	template <typename T> struct vls_allocator;

	template <typename T, typename Enabled = void>
	struct vls_traits
	{
		static bool const need_construct = true;
		static bool const need_destroy   = true;

		static void construct(
			typename vls_allocator<T>::pointer p,
			typename vls_allocator<T>::size_type stride,
			typename vls_allocator<T>::const_reference val);
		static void destroy(
			typename vls_allocator<T>::pointer p,
			typename vls_allocator<T>::size_type stride);
		static void copy(
			typename vls_allocator<T>::reference lhs,
			typename vls_allocator<T>::size_type stride,
			typename vls_allocator<T>::const_reference rhs);
	};

	template <typename T>
	struct vls_traits<T, typename std::enable_if<std::is_pod<T>::value>::type>
	{
		static bool const need_construct = false;
		static bool const need_destroy   = false;

		static void construct(
			typename vls_allocator<T>::pointer p,
			typename vls_allocator<T>::size_type /*stride*/,
			typename vls_allocator<T>::const_reference val)
		{
			new (static_cast<void*>(p)) T(val);
		}

		static void destroy(
			typename vls_allocator<T>::pointer /*p*/,
			typename vls_allocator<T>::size_type /*stride*/)
		{
		}

		static void copy(
			typename vls_allocator<T>::reference lhs,
			typename vls_allocator<T>::size_type /*stride*/,
			typename vls_allocator<T>::const_reference rhs)
		{
			lhs = rhs;
		}
	};

	template <typename T>
	struct vls_allocator
	{
		typedef T               value_type;
		typedef T*              pointer;
		typedef T const*        const_pointer;
		typedef T&              reference;
		typedef T const&        const_reference;
		typedef std::size_t     size_type;
		typedef std::ptrdiff_t  difference_type;
		typedef std::true_type  propagate_on_containter_move_assignment;

		template<typename U> struct rebind { typedef vls_allocator<U> other; };

		vls_allocator(size_t alignment = 1): align_(alignment)
		{
		}

		pointer allocate(size_type stride, size_type n, vls_allocator<void>::const_pointer hint = 0)
		{
			if(align_ == 1)
			{
				return reinterpret_cast<pointer>( ::operator new(stride * n) );
			}
			else
			{
				return reinterpret_cast<pointer>(aligned_malloc(stride * n, align_));
			}
		}

		void deallocate(pointer p, size_type stride, size_type n)
		{
			if(align_ == 1)
			{
				::operator delete(p);
			}
			else
			{
				aligned_free(reinterpret_cast<void*>(p));
			}
			
		}

		pointer address(reference val)
		{
			return &val;
		}

		const_pointer address(const_reference val) const
		{
			return &val;
		}

		void construct(pointer p, size_type stride, const_reference val)
		{
			vls_traits<value_type>::construct(p, stride, val);
		}

		void destroy(pointer p, size_type stride)
		{
			vls_traits<value_type>::destroy(p, stride);
		}

		size_type max_size(size_type stride) const
		{
			return std::numeric_limits<size_type>::max() / stride;
		}

	private:
		size_t align_;
	};

	template <>
	struct vls_allocator<void>
	{
		typedef void            value_type;
		typedef void*           pointer;
		typedef void const*     const_pointer;
		typedef std::size_t     size_type;
		typedef std::ptrdiff_t  difference_type;
	};

	template <typename VLSVectorT>
	struct vls_vector_iterator: public std::iterator<std::random_access_iterator_tag, typename VLSVectorT::value_type>
	{
	private:
		friend typename
			VLSVectorT;
		typedef vls_vector_iterator<VLSVectorT>
			this_type;
		value_type* p;
		size_t      stride;

		this_type(value_type* p, size_t stride): p(p), stride(stride)
		{
		}

	public:
		this_type(this_type const& rhs): p(rhs.p), stride(rhs.stride)
		{
		}

		this_type& operator = (this_type const& rhs)
		{
			p = rhs.p;
			stride = rhs.stride;
			return *this;
		}

		this_type& operator ++()
		{
			p = advance_bytes(p, stride);
			return *this;
		}

		this_type  operator ++(int)
		{
			return this_type(advance_bytes(p, stride), stride);
		}

		this_type& operator --()
		{
			p = advance_bytes(p, -stride);
			return *this;
		}

		this_type  operator --(int)
		{
			return this_type(advance_bytes(p, stride), -stride);
		}

		reference  operator *  ()
		{
			return *p;
		}

		pointer    operator -> ()
		{
			return p;
		}

		bool operator == (this_type const& rhs)
		{
			return p == rhs.p;
		}

		bool operator != (this_type const& rhs)
		{
			return p != rhs.p;
		}

		bool operator < (this_type const& rhs)
		{
			return p < rhs.p;
		}

		bool operator <= (this_type const& rhs)
		{
			return p <= rhs.p;
		}

		bool operator > (this_type const& rhs)
		{
			return p > rhs.p;
		}

		bool operator >= (this_type const& rhs)
		{
			return p >= rhs.p;
		}

		this_type& operator += (difference_type n)
		{
			p = advance_bytes(p, n * stride);
			return *this;
		}

		this_type& operator -= (difference_type n)
		{
			p = advance_bytes(p, -n * stride);
			return *this;
		}

		this_type operator + (difference_type n)
		{
			return this_type(advance_bytes(p, n * stride), stride);
		}

		this_type operator - (difference_type n)
		{
			return (*this) + (-n);
		}

		difference_type operator - (this_type const& rhs)
		{
			return distance_bytes(&rhs, this) / stride;
		}

		reference operator [] (difference_type index)
		{
			return *( (*this) + index );
		}
	};

	template <typename VLSVectorT>
	vls_vector_iterator<VLSVectorT> operator + (
		typename vls_vector_iterator<VLSVectorT>::difference_type n,
		vls_vector_iterator<VLSVectorT> const& lhs)
	{
		return lhs + n;
	}

	template <typename VLSVectorT>
	vls_vector_iterator<VLSVectorT> operator - (
		typename vls_vector_iterator<VLSVectorT>::difference_type n,
		vls_vector_iterator<VLSVectorT> const& lhs)
	{
		return lhs - n;
	}

	template <typename VLSVectorT>
	struct vls_vector_const_iterator: public std::iterator<std::random_access_iterator_tag, typename VLSVectorT::value_type const>
	{
	private:
		friend typename
			VLSVectorT;
		typedef vls_vector_const_iterator<VLSVectorT>
			this_type;
		pointer p;
		size_t  stride;

		this_type(pointer p, size_t stride): p(p), stride(stride)
		{
		}

	public:
		this_type(this_type const& rhs): p(rhs.p), stride(rhs.stride)
		{
		}

		vls_vector_iterator<VLSVectorT> make_mutable_iter()
		{
			return *reinterpret_cast(vls_vector_iterator<VLSVectorT>*)(this);
		}

		this_type& operator = (this_type const& rhs)
		{
			p = rhs.p;
			stride = rhs.stride;
			return *this;
		}

		this_type& operator ++()
		{
			p = advance_bytes(p, stride);
			return *this;
		}

		this_type  operator ++(int)
		{
			return this_type(advance_bytes(p, stride), stride);
		}

		this_type& operator --()
		{
			p = advance_bytes(p, -stride);
			return *this;
		}

		this_type  operator --(int)
		{
			return this_type(advance_bytes(p, stride), -stride);
		}

		reference  operator *  ()
		{
			return *p;
		}

		pointer    operator -> ()
		{
			return p;
		}

		bool operator == (this_type const& rhs)
		{
			return p == rhs.p;
		}

		bool operator != (this_type const& rhs)
		{
			return p != rhs.p;
		}

		bool operator < (this_type const& rhs)
		{
			return p < rhs.p;
		}

		bool operator <= (this_type const& rhs)
		{
			return p <= rhs.p;
		}

		bool operator > (this_type const& rhs)
		{
			return p > rhs.p;
		}

		bool operator >= (this_type const& rhs)
		{
			return p >= rhs.p;
		}

		this_type& operator += (difference_type n)
		{
			p = advance_bytes(p, n * stride);
			return *this;
		}

		this_type& operator -= (difference_type n)
		{
			p = advance_bytes(p, -n * stride);
			return *this;
		}

		this_type operator + (difference_type n)
		{
			return this_type(advance_bytes(p, n * stride), stride);
		}

		this_type operator - (difference_type n)
		{
			return (*this) + (-n);
		}

		difference_type operator - (this_type const& rhs)
		{
			return distance_bytes(rhs.p, p) / stride;
		}

		reference operator [] (difference_type index)
		{
			return *( (*this) + index );
		}
	};

	template <typename VLSVectorT>
	vls_vector_const_iterator<VLSVectorT> operator + (
		typename vls_vector_const_iterator<VLSVectorT>::difference_type n,
		vls_vector_const_iterator<VLSVectorT> const& lhs)
	{
		return lhs + n;
	}

	template <typename VLSVectorT>
	vls_vector_const_iterator<VLSVectorT> operator - (
		typename vls_vector_const_iterator<VLSVectorT>::difference_type n,
		vls_vector_const_iterator<VLSVectorT> const& lhs)
	{
		return lhs - n;
	}

	template <typename T, typename Alloc = vls_allocator<T>>
	class vls_vector
	{
	public:
		typedef vls_vector<T, Alloc>    this_type;
		typedef T                       value_type;
		typedef T*                      pointer;
		typedef T const*                const_pointer;
		typedef T&                      reference;
		typedef T const&                const_reference;
		typedef std::size_t             size_type;
		typedef std::ptrdiff_t          difference_type;
		typedef vls_vector_iterator<this_type> 
			iterator;
		typedef vls_vector_const_iterator<this_type>
			const_iterator;

		friend void swap(this_type& lhs, this_type& rhs);

		vls_vector(size_type stride):
			elems_(nullptr), capacity_(0), size_(0), stride_(stride)
		{
		}

		vls_vector(size_type stride, size_type n):
			elems_(nullptr), capacity_(0), size_(0), stride_(stride)
		{
			resize(n);
		}

		vls_vector(this_type const& rhs):
			elems_(nullptr), capacity_(0), size_(0), stride_(rhs.stride_)
		{
			reserve(rhs.size_);
			size_ = rhs.size();
			auto src = rhs.begin();
			auto dst = begin();
			for(; src != rhs.end(); ++src, ++dst)
			{
				vls_traits<T>::copy(*dst, stride_, *src);
			}
		}

		~vls_vector()
		{
			clear();
			alloc_.deallocate(elems_, stride_, capacity_);
		}

		bool empty() const
		{
			return size() == 0;
		}

		size_type size() const
		{
			return size_;
		}

		size_type stride() const
		{
			return stride;
		}

		void clear()
		{
			if( begin() != end() )
			{
				destroy_range<T>( begin(), end() );
				size_ = 0;
			}
		}

		iterator erase(const_iterator it0, const_iterator it1)
		{
			if( it0 == begin() && it1 == end() )
			{
				clear();
			}
			else if(it0 != it1)
			{
				auto iter_beg = it0.make_mutable_iter();
				auto iter_end = it1.make_mutable_iter();

				destroy_range(it_beg, it_end);
				auto new_iter_end = std::move(it_end, end(), it_beg);
				size_ = static_cast<size_type>(new_iter_end - begin());
				return iter_beg;
			}
		}

		void reserve(size_type cap)
		{
			if (cap > capacity_)
			{
				T* new_elems = alloc_.allocate(stride_, cap);
				auto src = begin();
				auto dst = iterator(new_elems, stride_);
				for(; src != end(); ++src, ++dst) vls_traits<T>::copy(*dst, stride_, *src);
				alloc_.deallocate(elems_, stride_, capacity_);
				elems_ = new_elems;
				capacity_ = cap;
			}
		}

		void resize(size_type sz)
		{
			if (sz == size_)
			{
				return;
			}

			if (sz < size_)
			{
				erase( begin() + sz, end() );
			}
			else
			{
				reserve(sz);
				auto construct_beg = end();
				size_ = sz;
				construct_range( construct_beg, end() );
			}
		}

		void push_back(const_reference v)
		{
			if(size_ == capacity_)
			{
				reserve(capacity_ + capacity_ / 2 + 4);
			}
			(*this)[size_++] = v;
		}

		void pop_back()
		{
			if( !empty() )
			{
				--size_;
			}
		}

		reference back()
		{
			return (*this)[size_-1];
		}

		const_reference back() const
		{
			return (*this)[size_];
		}

		reference operator [] (size_type index)
		{
			return *advance_bytes(elems_, stride_*index);
		}

		const_reference operator[] (size_type index) const
		{
			return *advance_bytes(elems_, stride_*index);
		}

		iterator begin()
		{
			return iterator(elems_, stride_);
		}

		iterator end()
		{
			return iterator(&(*this)[size_], stride_);
		}

		const_iterator begin() const
		{
			return const_iterator(elems_, stride_);
		}

		const_iterator end() const
		{
			return const_iterator(&(*this)[size_], stride_);
		}

	private:
		template <typename U>
		void construct_range(iterator it0, iterator it1, typename std::enable_if< vls_traits<U>::need_construct>::type* = nullptr)
		{
			for(; it0 != it1; ++it0)
			{
				alloc_.construct(*it0, stride_);
			}
		}

		template <typename U>
		void construct_range(iterator,     iterator,     typename std::enable_if<!vls_traits<U>::need_construct>::type* = nullptr)
		{
		}

		template <typename U>
		void destroy_range  (iterator it0, iterator it1, typename std::enable_if< vls_traits<U>::need_destroy  >::type* = nullptr)
		{
			for(; it0 != it1; ++it0)
			{
				alloc_.destroy(&(*it0), stride_);
			}
		}

		template <typename U>
		void destroy_range  (iterator,     iterator,     typename std::enable_if<!vls_traits<U>::need_destroy  >::type* = nullptr)
		{
		}

		size_type stride_;
		size_type size_;
		size_type capacity_;
		T*        elems_;
		Alloc     alloc_;
	};

	template <typename T, typename Alloc>
	void swap(vls_vector<T, Alloc>& lhs, vls_vector<T, Alloc>& rhs)
	{
		std::swap(lhs.stride_, rhs.stride_);
		std::swap(lhs.size_, rhs.size_);
		std::swap(lhs.capacity_, rhs.capacity_);
		std::swap(lhs.elems_, rhs.elems_);
		std::swap(lhs.alloc_, rhs.alloc_);
	}
};
