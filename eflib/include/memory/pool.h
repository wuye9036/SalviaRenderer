#ifndef EFLIB_MEMORY_POOL_H
#define EFLIB_MEMORY_POOL_H

#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/mpl/if.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <cassert>
#include <limits>
#include <vector>

namespace eflib{
	namespace pool
	{
		template <typename ObjectT, int MaxCount, bool IsFreeTogether = true>
		class stack_pool{
		public:
			stack_pool(){
				initialize_usage();
			}

			void* malloc(){
				return malloc_impl(p);
			}

			void free( void* p ){
				free_impl(p);
			}

			~stack_pool(){
			}

		protected:
			void intialize_usage( typename boost::disable_if_c<IsFreeTogether>::type* dummy = 0 ){
				usage[0] = false;
				memcpy( boost::addressof(usage[0]), usage[0], sizeof(usage) );
			}

			void* malloc_impl(){
				for ( int i = 0; i < MaxCount; ++i ){
					if ( usage[i] == false ){
						usage[i] = true;
						return boost::addressof( data[ObjectSize*i] );
					}
				}
				return NULL;
			}

			void free_impl( void* const p ){
				::assert( is_from_pool(p) );
				intptr_t pos = ( (intptr_t)p - (intptr_t)boost::addressof(data[0]) ) / ObjectSize;
				usage[pos] = false;
			}

			bool is_from_pool(void* const p){
				intptr_t diff = (intptr_t)p - (intptr_t)boost::addressof(data[0]);
				if ( diff >= 0 && diff % ObjectSize == 0 && ( diff / ObjectSize < MaxCount ) ){
					return true;
				}
			}

			static const int ObjectSize = sizeof(ObjectT);
			boost::array<bool, MaxCount> usage;
			char data[ObjectSize * MaxCount];
		};

		template <typename ObjectT, int MaxCount>
		class stack_pool<ObjectT, MaxCount, true>
		{
		private:
			static const size_t ObjectSize = sizeof(ObjectT);
			unsigned char data[ObjectSize * MaxCount];
			size_t usage;

		public:
			stack_pool()
			{
				initialize_usage();
			}

			void* malloc()
			{
				return malloc_impl();
			}

			void free(void* p)
			{
				free_impl(p);
			}

			~stack_pool(){
			}

		private:
			void initialize_usage()
			{
				usage = 0;
			}

			void* malloc_impl()
			{
				assert(usage < MaxCount);
				void* ret = boost::addressof(data[ObjectSize*usage]);
				++usage;
				return ret;
			}

			void free_impl(void* const p)
			{
				return;
			}
		};
	
		template <typename T>
		class preserved_pool
		{
		public:
			preserved_pool(): data_mem(NULL), sz(0), cap(0)
			{}

			~preserved_pool()
			{
				::free(data_mem);
			}

			void reserve(size_t capacity)
			{
				if(data_mem == NULL)
				{
					data_mem = static_cast<T*>(::malloc(sizeof(T)*capacity));
					sz = 0;
					cap = capacity;
				}
				assert(capacity <= cap);
			}

			T* alloc()
			{
				++sz;
#if defined(EFLIB_DEBUG)
				if(sz >= cap)
				{
					assert(false);
					return NULL;
				}
#endif
				return data_mem + sz;
			}

			void dealloc(T*) {}

			T* begin() const
			{
				return data_mem;
			}

			T* end() const
			{
				return data_mem + sz;
			}
		private:
			preserved_pool(preserved_pool const&);
			preserved_pool& operator = (preserved_pool const&);

			size_t	sz;
			size_t	cap;
			T*		data_mem;
		};
	}
}

#endif