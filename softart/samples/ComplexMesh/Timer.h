#ifndef _TIMER_H
#define _TIMER_H

#pragma once

#include <limits>
#ifdef EFLIB_WINDOWS
	#define NOMINMAX
	#include <windows.h>
#else
	#include <ctime>
	#include <limits>
#endif

class Timer
{
public:
	Timer()
	{
		if (0 == cps_)
		{
#ifdef EFLIB_WINDOWS
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			cps_ = static_cast<uint64_t>(frequency.QuadPart);
#else
			cps_ = CLOCKS_PER_SEC;
#endif
		}

		this->restart();
	} // postcondition: elapsed()==0
	void restart()
	{
		start_time_ = this->current_time();
	} // postcondition: elapsed()==0

	// return elapsed time in seconds
	double elapsed() const
	{
		return this->current_time() - start_time_;
	}

	// return estimated maximum value for elapsed()
	double elapsed_max() const
	{
#ifdef EFLIB_WINDOWS
		return static_cast<double>(std::numeric_limits<uint64_t>::max()) / cps_ - start_time_;
#else
		return static_cast<double>(std::numeric_limits<std::clock_t>::max()) / cps_ - start_time_;
#endif
	}

	// return minimum value for elapsed()
	double elapsed_min() const
	{
		return 1.0 / cps_;
	}

	double current_time() const
	{
#ifdef EFLIB_WINDOWS
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return static_cast<double>(count.QuadPart) / cps_;
#else
		return static_cast<double>(std::clock()) / cps_;
#endif
	}

private:
	double start_time_;

	static uint64_t cps_;
};

#endif		// _TIMER_H
