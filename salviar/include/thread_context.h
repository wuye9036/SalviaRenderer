#pragma once

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

struct thread_context
{
	int32_t					item_count;
	boost::atomic<int32_t>*	package_token;
	int32_t					package_size;
};

END_NS_SALVIAR();