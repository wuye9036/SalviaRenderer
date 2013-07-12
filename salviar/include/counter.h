#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/shared_declaration.h>

#include <atomic>
#include <future>

BEGIN_NS_SALVIAR();

class counter
{
public:
	uintptr_t get()
	{
		return future_.get();
	}
	
	bool wait_for()
	{
		return future_.wait_for( std::chrono::seconds(0) ) == std::future_status::ready;
	}
	
	void begin()
	{
		value_ = 0;
		future_ = promise_.get_future();
	}
	
	void end()
	{
		promise_.set_value(value_);
	}
	
	void add_count(uintptr_t op)
	{
		value_ += op;
	}

private:
	std::atomic<uintptr_t>	value_;
	std::future<uintptr_t>	future_;
	std::promise<uintptr_t>	promise_;
};

#define SALVIA_ENABLE_PERFORMANCE_COUNTER

#if defined(SALVIA_ENABLE_PERFORMANCE_COUNTER)
inline void add_count(counter* cnt, uint64_t op)
{
	cnt->add_count(op);
}
#else
inline void add_count(counter* /*cnt*/, uint64_t /*op*/)
{
	// Do nothing
}
#endif

END_NS_SALVIAR();

/*
TODO Related API.
void renderer::begin(counter_ptr const& cnt)
{
	counter_set.insert(cnt);
}

void renderer::end(counter_ptr const& cnt)
{
	counter_set.erase(cnt);
}

bool renderer::get_data(uintptr_t& data, counter_ptr const& cnt, bool do_not_wait)
{
	while( !cnt->wait_for() )
	{
		if(do_not_wait)
		{
			return false;
		}
	}
	data = cnt->get();
	return true;
}

counter_ptr cnt = renderer->create_counter();
renderer->begin(cnt);
renderer->end(cnt);
renderer->get_data(cnt);
*/
