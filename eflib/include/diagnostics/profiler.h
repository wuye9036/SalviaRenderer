#ifndef EFLIB_DIAGNOSTICS_PROFILER_H
#define EFLIB_DIAGNOSTICS_PROFILER_H

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
	};
	
	class profiler
	{
	public:
		void start	(char const* name, size_t tag);
		void end	(char const* name);
		profiling_item const* root() const;
	private:
		profiling_item 	root;
		profiling_item* current;
	}
}

#endif