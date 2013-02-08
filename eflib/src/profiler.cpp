#include <eflib/include/diagnostics/profiler.h>
#include <eflib/include/utility/unref_declarator.h>

#include <cassert>
#include <iostream>
#include <algorithm>

using std::string;
using std::vector;
using std::cout;
using std::endl;

namespace chrono = boost::chrono;

namespace eflib
{
	profiling_item::profiling_item()
		: duration_(0.0), tag(0), parent(NULL)
	{
	}

	profiling_item::~profiling_item()
	{
		for(size_t i = 0; i < children.size(); ++i)
		{
			delete children[i];
		}
	}

	void profiling_item::start(profiling_item::clock::time_point start_time)
	{
		start_time_ = start_time;
	}

	void profiling_item::end(profiling_item::clock::time_point end_time)
	{
		typedef chrono::duration< double, boost::ratio<1> > dseconds;
		profiling_item::duration_ +=
			chrono::duration_cast<dseconds>(end_time - start_time_).count();
	}

	double profiling_item::duration() const
	{
		return duration_;
	}

	double profiling_item::children_duration() const
	{
		double ret = 0.0;
		for(size_t i = 0; i < children.size(); ++i)
		{
			ret += children[i]->duration();
		}
		return ret;
	}

	double profiling_item::exclusive_duration() const
	{
		return duration() - children_duration();
	}

	bool profiling_item::try_merge(profiling_item const* rhs)
	{
		assert(parent == rhs->parent);
		assert(this != rhs);
		if(name == rhs->name && tag == rhs->tag)
		{
			children.insert(children.end(), rhs->children.begin(), rhs->children.end());
			duration_ += rhs->duration_;
			return true;
		}
		return false;
	}

	profiler::profiler(): current_(NULL)
	{
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
			current_->children.push_back( new profiling_item() );
			current_ = current_->children.back();
		}

		current_->name = name;
		current_->tag = tag;
		current_->start( profiling_item::clock::now() );
		current_->parent = parent;
	}

	void profiler::end(string const& name)
	{
		EFLIB_UNREF_DECLARATOR(name);
		assert(name == current_->name);
		current_->end( profiling_item::clock::now() );
		current_ = current_->parent;
	}

	void merge_children(profiling_item* parent)
	{
		// Merge children items of parent.
		size_t unique_count = parent->children.size();
		for(size_t i = 0; i < unique_count; ++i)
		{
			profiling_item* processed_item = parent->children[i];

			size_t i_processing = i+1;
			while(i_processing < unique_count)
			{
				profiling_item* processing_item = parent->children[i_processing];
				if( processed_item->try_merge(processing_item) )
				{
					std::swap(parent->children[i_processing], parent->children.back());
					delete parent->children.back();
					parent->children.pop_back();
					--unique_count;
				}
				else
				{
					++i_processing;
				}
			}
		}

		// Merge recursively.
		for(size_t i = 0; i < unique_count; ++i)
		{
			merge_children(parent->children[i]);
		}
	}

	void profiler::merge_items()
	{
		merge_children(&root_);
	}

	profiling_item const* profiler::root() const
	{
		return &root_;
	}

	void visit_profiling_item_recursively(
		profiling_item const* item, size_t level, size_t max_level
		)
	{
		assert(level < 40);

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
		char is_string[inclusive_ms_width];
		std::fill_n(is_string, inclusive_ms_width, ' ');
		_snprintf( is_string, inclusive_ms_width, "%8.3f", item->duration() );
		std::copy(is_string, is_string+inclusive_ms_width, line+inclusive_ms_offset);

		// Print exclusive seconds.
		char es_string[exclusive_ms_width];
		std::fill_n(es_string, exclusive_ms_width, ' ');
		_snprintf( es_string, exclusive_ms_width, "%8.3f", item->exclusive_duration() );
		std::copy(es_string, es_string+exclusive_ms_width, line+exclusive_ms_offset);

		// Print percentage
		double percent = 1.0f;
		if(item->parent != NULL && item->parent->duration() > 0)
		{
			percent = item->duration() / item->parent->duration();
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
				item->children[i_child], level+1, max_level
				);
		}

		return;
	}

	void print_profiler(profiler const* prof, size_t max_level)
	{
		cout << " --- Profiling Result BEG --- " << endl;
		cout << "                      Name                            Secs(I)   Secs(E)    %  " << endl;
		visit_profiling_item_recursively(prof->root(), 0, max_level);
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