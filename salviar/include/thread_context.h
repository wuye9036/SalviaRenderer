#pragma once

#include <salviar/include/salviar_forward.h>
#include <salviar/include/thread_pool.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/atomic/atomic.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

struct thread_context
{
	class package_cursor
	{
	private:
		friend struct thread_context;

		thread_context const*	owner;
		int32_t					cur;

		package_cursor(thread_context const* owner, int32_t cur)
			: owner(owner), cur(cur)
		{
		}
	public:
		package_cursor(): owner(NULL), cur(0)
		{
		}

		package_cursor(package_cursor const& rhs)
			: owner(rhs.owner), cur(rhs.cur)
		{
		}

		package_cursor& operator = (package_cursor const& rhs)
		{
			owner = rhs.owner;
			cur = rhs.cur;
			return *this;
		}

		int32_t package_index() const
		{
			return cur;
		}

		std::pair<int32_t, int32_t> item_range() const
		{
			int32_t beg = cur * owner->package_size;
			int32_t end = std::min(beg + owner->package_size, owner->item_count);
			return std::make_pair(beg, end);
		}

		bool valid() const
		{
			return cur < owner->package_count;
		}
	};

	size_t					thread_id;

	int32_t					item_count;
	boost::atomic<int32_t>*	working_package_id;
	int32_t					package_size;
	int32_t					package_count;

	static int32_t compute_package_count(int32_t item_count, int32_t package_size)
	{
		return (item_count + package_size - 1) / package_size;
	}

	package_cursor next_package() const
	{
		return package_cursor(this, (*working_package_id)++);
	}
};

inline void init_thread_context(
	thread_context* ctxts, size_t N,
	int32_t item_count, boost::atomic<int32_t>* working_package_id, int32_t package_size)
{
	int32_t package_count = thread_context::compute_package_count(item_count, package_size);

	for(size_t i = 0; i < N; ++i)
	{
		ctxts[i].thread_id			= i;
		ctxts[i].item_count			= item_count;
		ctxts[i].working_package_id = working_package_id;
		ctxts[i].package_size		= package_size;
		ctxts[i].package_count		= package_count;
	}
}

template <typename ThreadFuncT> // ThreadFuncT = function <void (thread_context const*)>
inline void execute_threads(ThreadFuncT const& fn, int32_t item_count, int32_t package_size, int32_t thread_count)
{
	// Compute package information
	boost::atomic<int32_t>				working_package(0);
	boost::shared_array<thread_context>	thread_ctxts(new thread_context[thread_count]);

	// Initialize contexts per thread
	thread_context* pctxts = thread_ctxts.get();
	init_thread_context(thread_ctxts.get(), thread_count, item_count, &working_package, package_size);

	// Call functions by context.
	for (size_t i = 0; i < thread_count - 1; ++ i)
	{
		global_thread_pool().schedule( [&fn, pctxts, i] () { fn(&pctxts[i]); } );
	}
	fn(&pctxts[thread_count-1]);

	global_thread_pool().wait();
}

template <typename ThreadFuncT> // ThreadFuncT = function <void (thread_context const*)>
inline void execute_threads(ThreadFuncT const& fn, int32_t item_count, int32_t package_size)
{
	int32_t thread_count = static_cast<int32_t>(eflib::num_available_threads());
	execute_threads(fn, item_count, package_size, thread_count);
}

END_NS_SALVIAR();
