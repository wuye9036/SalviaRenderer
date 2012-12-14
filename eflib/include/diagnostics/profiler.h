#ifndef EFLIB_DIAGNOSTICS_PROFILER_H
#define EFLIB_DIAGNOSTICS_PROFILER_H

#include <eflib/include/platform/typedefs.h>

#include <string>
#include <vector>

namespace eflib
{
	class profiling_item
	{
	public:
		size_t						tag;
		std::string					name;
		uint64_t					start_time;
		uint64_t					end_time;
		std::vector<profiling_item>	children;
		profiling_item*				parent;
	};
	
	class profiler
	{
	public:
		profiler();

		void start	(std::string const&, size_t tag);
		void end	(std::string const&);

		profiling_item const* root() const;
		uint64_t			  cps() const;

	private:
		uint64_t		cps_;
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