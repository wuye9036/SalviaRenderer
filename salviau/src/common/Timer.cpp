#include <eflib/include/platform/typedefs.h>
#include <salviau/include/common/timer.h>

BEGIN_NS_SALVIAU();

timer::timer()
{
	restart();
}

void timer::restart()
{
	start_time_ = current_time();
}

double timer::elapsed() const
{
	seconds sec = current_time() - start_time_;
	return sec.count();
}

timer::time_point timer::current_time() const
{
	return clock_type::now();
}

END_NS_SALVIAU();