#pragma once

#ifndef EFLIB_DIAGNOSTICS_PROFILER_H
#define EFLIB_DIAGNOSTICS_PROFILER_H

#include <eflib/include/platform/typedefs.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/chrono.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

namespace eflib
{
	class profiling_item
	{
	public:
		typedef boost::chrono::high_resolution_clock clock;

		profiling_item();
		~profiling_item();
		
		void start(clock::time_point start_time);
		void end(clock::time_point end_time);

		double	duration() const;
		double	children_duration() const;
		double	exclusive_duration() const;

		bool	try_merge(profiling_item const* rhs);
		
		size_t							tag;
		std::string						name;
		
		std::vector<profiling_item*>	children;
		profiling_item*					parent;

	private:
		clock::time_point	start_time_;
		double				duration_;
	};
	
	class profiler
	{
	public:
		profiler();

		void start	(std::string const&, size_t tag);
		void end	(std::string const&);

		void merge_items();

		profiling_item const* root() const;

	private:
		profiling_item 	root_;
		profiling_item* current_;
	};

	class profiling_scope
	{
	public:
		profiling_scope(profiler* prof, std::string const& name, size_t tag = 0);
		~profiling_scope();
	private:
		profiling_scope(const profiling_scope&);
		profiling_scope& operator = (const profiling_scope&);
		profiler*	prof;
		std::string name;
	};

	void print_profiler(profiler const* prof, size_t max_level);
}

#endif