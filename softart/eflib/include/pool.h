#ifndef EFLIB_POOL_H
#define EFLIB_POOL_H

#include "detail/typedefs.h"

#include <boost/type_traits.hpp>

#include <limits>
#include <vector>

namespace efl{
	namespace pool{
		//threading modeling
		struct single_thread
		{
			//volatile type
			template<class T> struct volatile_type{
				typedef T result;
			};

			//atom operations
			template<class T>
			T increase(const T& v){
				return v+1;
			}

			template<class T>
			T decrease(const T& v){
				return v-1;
			}

			template<class T>
			T increase(T& v){
				return v++;
			}

			template<class T>
			T decrease(T& v){
				return v--;
			}

			template<class T>
			T add(T& v, const T& inc){
				T rv(v);
				v += inc;
				return rv;
			}

			template<class Functor>
			typename Functor::result_type atom_operation(const Functor& functor)
			{
				return functor();
			}

			//lock strategies
			void lock(){
			}

			void unlock(){
			}

			struct scoped_lock{
				scoped_lock(){
				}
				~scoped_lock(){
				}
			};
		};

		// allocator strategies
		//alloc: O(1) free: No Cost
		template<size_t capacity, class ThreadingModel, class Storage>
		struct unsafe_release : private ThreadingModel
		{
			typedef ThreadingModel TM;
			typedef typename TM::volatile_type<int64_t>::result cursor_type;

			unsafe_release(size_t elem_size)
				: pool_(capacity*elem_size),
				cursor_(0),
				elem_size_(elem_size)
			{}

			void* alloc(){
				cursor_type rv_cur = increase(cursor_);
				if(rv_cur >= capacity){
					add(rv_cur, int64_t(-capacity));
				}
				return &pool_[(cursor % capacity) * elem_size_];
			}

			void free(void *const pointer){
			}

		private:
			int64_t elem_size_;

			Storage pool_;
			cursor_type cursor_;
		};

		template<size_t capacity, class ThreadingModel, class Storage>
		struct unsafe_debug : private ThreadingModel
		{
			//如果直接写的话无法通过编译
			typedef ThreadingModel TM;
			typedef typename TM::volatile_type<int64_t>::result cursor_type;

			unsafe_debug(size_t elem_size)
				:pool_(capacity*elem_size),
				cursor_(0),
				is_used_(capacity, false),
				elem_size_(elem_size)
			{
			}

			void* alloc(){
				scoped_lock lock;
				cursor_ %= capacity;
				if(is_used_[size_t(cursor_)]){
					custom_assert(false, "the pool crashed!");
					return NULL;
				}
				is_used_[size_t(cursor_)] = true;
				return &pool_[(size_t(cursor_++))*size_t(elem_size_)];
			}

			void free(void *const pointer){
				size_t diff = size_t(pointer) - size_t(&pool_[0]);

				if(diff % elem_size_ != 0){
					custom_assert(false, "Error Pointer For Free!");
					return;
				}

				size_t idx = diff / size_t(elem_size_);
				if(idx >= capacity){
					custom_assert(false, "Error Pointer For Free!");
					return;
				}

				ThreadingModel::scoped_lock lock;
				is_used_[idx] = false;
			}

		private:
			int64_t elem_size_;
			Storage pool_;
			std::vector<bool> is_used_;
			cursor_type cursor_;
		};

		struct safe;

		//capacity
		const size_t unlimit = std::numeric_limits<size_t>::max();
	}

	template<
		size_t capacity,
		template<size_t, class, class> class AllocStrategy = pool::unsafe_release,
		class ThreadingModel = pool::single_thread,
		class Storage = std::vector<char>
	>
	struct recycled_pool : private AllocStrategy<capacity, ThreadingModel, Storage>
	{
		typedef AllocStrategy<capacity, ThreadingModel, Storage> AllocType;
		recycled_pool(size_t elem_size):AllocStrategy<capacity, ThreadingModel, Storage>(elem_size){}

		void* malloc()
		{
			return AllocType::alloc();
		}

		void free(void* const pointer)
		{
			return AllocType::free(pointer);
		}
	};

	//Partial spec for unlimit capacity pool, Unimplamented 
	//template<size_t capaticy, class AllocStrategy, class ThreadModel>
	//struct recycled_pool<pool::unlimit, AllocStrategy, ThreadModel>
	//{
	//};
}

#endif