#ifndef EFLIB_POOL_H
#define EFLIB_POOL_H

#include "detail/typedefs.h"
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/mpl/if.hpp>
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6385)
#endif
#include <boost/array.hpp>
#ifdef EFLIB_MSVC
#pragma warning(push)
#endif
#include <cassert>
#include <limits>
#include <vector>

namespace eflib{
	namespace pool{
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
		class stack_pool <ObjectT, MaxCount, true>{
				public:
			stack_pool(){
				initialize_usage();
			}

			void* malloc(){
				return malloc_impl();
			}

			void free( void* p ){
				free_impl(p);
			}

			~stack_pool(){
			}

		protected:
			void initialize_usage( ){
				usage = 0;
			}

			void* malloc_impl(){
				assert( usage < MaxCount );
				return boost::addressof( data[ObjectSize*(usage++)] );
			}

			void free_impl( void* const p){
				return;
			}

			static const int ObjectSize = sizeof(ObjectT);
			int usage;
			char data[ObjectSize * MaxCount];
		};
	}
}

#endif