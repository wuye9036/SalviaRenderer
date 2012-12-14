#include <eflib/include/diagnostics/profiler.h>
#include <eflib/include/utility/unref_declarator.h>

#include <limits>
#if defined(EFLIB_WINDOWS)
	#define NOMINMAX
	#include <windows.h>
#else
	#include <ctime>
	#include <limits>
#endif

#include <cassert>
#include <iostream>
#include <algorithm>

using std::string;
using std::vector;
using std::cout;
using std::endl;

namespace eflib
{
	uint64_t current_counter()
	{
#if defined(EFLIB_WINDOWS)
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return static_cast<uint64_t>(count.QuadPart);
#else
		return static_cast<uint64_t>(std::clock());
#endif
	}

	profiler::profiler(): current_(NULL)
	{
#ifdef EFLIB_WINDOWS
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		cps_ = static_cast<uint64_t>(frequency.QuadPart);
#else
		cps_ = CLOCKS_PER_SEC;
#endif
	}

	void profiler::start(string const& name, size_t tag)
	{
		profiling_item* parent = current_;

		if(!current_)
		{
			current_ = &root_;
		}
		else
		{
			current_->children.push_back( profiling_item() );
			current_ = &current_->children.back();
		}

		current_->name = name;
		current_->tag = tag;
		current_->start_time = current_counter();
		current_->end_time = current_->start_time;
		current_->parent = parent;
	}

	void profiler::end(string const& name)
	{
		EFLIB_UNREF_DECLARATOR(name);
		assert(name == current_->name);
		current_->end_time = current_counter();
		current_ = current_->parent;
	}

	profiling_item const* profiler::root() const
	{
		return &root_;
	}

	uint64_t profiler::cps() const
	{
		return cps_;
	}

	void visit_profiling_item_recursively(
		profiling_item const* item, size_t level, size_t max_level, uint64_t cps, uint64_t parent_ticks
		)
	{
		assert(level < 40);

		uint64_t inclusive_ticks = item->end_time - item->start_time;
		uint64_t nested_used_ticks = 0;
		for(size_t i_child = 0; i_child < item->children.size(); ++i_child)
		{
			nested_used_ticks += (item->children[i_child].end_time - item->children[i_child].start_time);
		}

		if(level > max_level)
		{
			return;
		}

		// Print item.
		// 80 char width,
		// Name,  Inclusive ms Exclusive Seconds, Inclusive Percentage.
		//  60         8              8                  4

		size_t indent = level * 2;
		int const line_width			= 79;

		int const name_width			= 53;
		int const inclusive_ms_width	= 10;
		int const exclusive_ms_width	= 10;
		int const percent_width			= 6;

		int const name_offset			= static_cast<int>(indent);
		int const inclusive_ms_offset	= name_width;
		int const exclusive_ms_offset	= inclusive_ms_offset + inclusive_ms_width;
		int const percent_offset		= exclusive_ms_offset + exclusive_ms_width;

		char line[line_width + 1];
		std::fill_n(line, line_width, ' ');
		line[line_width] = '\0';

		// Print name
		size_t name_length = item->name.length();
		if (name_length > name_width - indent)
		{
			// 'too_lang_name' -> 'too_lan...'
			size_t elited_name_length = name_width - indent - 3;
			std::copy(item->name.begin(), item->name.begin()+elited_name_length, line + name_offset);
			std::fill(line+name_width-3, line+name_width, '.');
		}
		else
		{
			std::copy(item->name.begin(), item->name.end(), line+name_offset);
		}

		// Print inclusive seconds.
		double inclusive_second = inclusive_ticks / (double)cps;
		char is_string[inclusive_ms_width];
		std::fill_n(is_string, inclusive_ms_width, ' ');
		_snprintf(is_string, inclusive_ms_width, "%8.3f", inclusive_second);
		std::copy(is_string, is_string+inclusive_ms_width, line+inclusive_ms_offset);

		// Print exclusive seconds.
		double exclusive_second = (inclusive_ticks-nested_used_ticks) / (double)cps;
		char es_string[exclusive_ms_width];
		std::fill_n(es_string, exclusive_ms_width, ' ');
		_snprintf(es_string, exclusive_ms_width, "%8.3f", exclusive_second);
		std::copy(es_string, es_string+exclusive_ms_width, line+exclusive_ms_offset);

		// Print percentage
		double percent = 1.0f;
		if(parent_ticks > 0)
		{
			percent = inclusive_ticks / (double)parent_ticks;
		}
		char pc_string[percent_width];
		std::fill_n(pc_string, percent_width, ' ');
		_snprintf(pc_string, percent_width, "%6.2f", percent * 100.0);
		std::copy(pc_string, pc_string+percent_width, line+percent_offset);

		std::replace(line, line+line_width, '\0', ' ');
		cout << line << endl;

		for(size_t i_child = 0; i_child < item->children.size(); ++i_child)
		{
			visit_profiling_item_recursively(
				&(item->children[i_child]), level+1, max_level, cps, inclusive_ticks
				);
		}

		return;
	}

	void print_profiler(profiler const* prof, size_t max_level)
	{
		cout << " --- Profiling Result BEG --- " << endl;
		cout << "                      Name                            Secs(I)   Secs(E)    %  " << endl;
		visit_profiling_item_recursively(prof->root(), 0, max_level, prof->cps(), 0);
		cout << " --- Profiling Result END --- " << endl;
	}

	profiling_scope::profiling_scope(profiler* prof, std::string const& name, size_t tag)
	{
		this->prof = prof;
		this->name = name;
		prof->start(name, tag);
	}

	profiling_scope::~profiling_scope()
	{
		prof->end(name);
	}
}