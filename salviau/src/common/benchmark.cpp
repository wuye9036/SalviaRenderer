#include <salviau/include/common/benchmark.h>

#include <salviar/include/renderer.h>
#include <salviar/include/texture.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <eflib/include/platform/boost_end.h>

#if defined(EFLIB_WINDOWS)
#include <Windows.h>
#endif

using namespace eflib;
using namespace salviar;
using namespace salviax::resource;
using namespace boost::property_tree;

static int const OUTPUT_PROFILER_LEVEL = 3;

BEGIN_NS_SALVIAU();

benchmark::benchmark(std::string const& benchmark_name)
{
	benchmark_name_ = benchmark_name;
	
#if defined(EFLIB_WINDOWS)
	HANDLE process_handle = GetCurrentProcess();
	SetPriorityClass(process_handle, HIGH_PRIORITY_CLASS);
#endif
}

benchmark::~benchmark()
{
	print_profiler(&prof_, OUTPUT_PROFILER_LEVEL);
}

void benchmark::begin_bench()
{
	prof_.start(benchmark_name_, 0);
}

void benchmark::end_bench()
{
	prof_.end(benchmark_name_);
	prof_.merge_items();
}

void benchmark::begin_frame()
{
	if(!pipeline_stat_obj_)
	{
		pipeline_stat_obj_ = renderer_->create_query(async_object_ids::pipeline_statistics);
		internal_stat_obj_ = renderer_->create_query(async_object_ids::internal_statistics);
		pipeline_prof_obj_ = renderer_->create_query(async_object_ids::pipeline_profiles);
	}

	renderer_->begin(pipeline_stat_obj_);
	renderer_->begin(internal_stat_obj_);
	renderer_->begin(pipeline_prof_obj_);
}

void benchmark::profiling(std::string const& stage_name, std::function<void()> const& fn)
{
	// std::cout << "[PROFILING] " << stage_name << std::endl;

	prof_.start(stage_name, 0);
	fn();
	prof_.end(stage_name);
}

void benchmark::end_frame()
{
    renderer_->end(pipeline_stat_obj_);
    renderer_->end(internal_stat_obj_);
	renderer_->end(pipeline_prof_obj_);

	frame_data frame_prof;

	renderer_->get_data(pipeline_stat_obj_, &frame_prof.pipeline_stat, false);
	renderer_->get_data(internal_stat_obj_, &frame_prof.internal_stat, false); 
	renderer_->get_data(pipeline_prof_obj_, &frame_prof.pipeline_prof, false);

	frame_profs_.push_back(frame_prof);
}

template <typename T>
void min_max(T& in_out_min, T& in_out_max, T v)
{
	if(v < in_out_min)
	{
		in_out_min = v;
	}
	else if (v > in_out_max)
	{
		in_out_max = v;
	}
}

template <typename ValueT, typename IterT, typename TransformT>
void reduce_and_output(IterT beg, IterT end, TransformT trans, ptree& parent, std::string const& path)
{
	ValueT minv;
	ValueT maxv;
	ValueT total = 0;
	ValueT avg = 0;

	if(beg == end)
	{
		minv = maxv = total = avg = 0;
	}
	else
	{
		auto it = beg;
		minv = maxv = total = trans(*it);
		++it;

		size_t count = 1;
		for(; it != end; ++it)
		{
			auto v = trans(*it);
			min_max(minv, maxv, v);
			total += v;
			++count;
		}

		avg = total / count;
	}

	ptree vnode;
	vnode.put("min", minv);
	vnode.put("max", maxv);
	vnode.put("total", total);
	vnode.put("avg", avg);
	parent.put_child(path, vnode);
}

void benchmark::save_results(std::string const& file_name)
{
	auto root = make_ptree(&prof_, OUTPUT_PROFILER_LEVEL);

	// Statistic
	root.put( "frames", total_frames() );

	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.cinvocations			;}, root, "async.pipeline_stat.cinvocations");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.cprimitives			;}, root, "async.pipeline_stat.cprimitives");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.ia_primitives		;}, root, "async.pipeline_stat.ia_primitives");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.ia_vertices			;}, root, "async.pipeline_stat.ia_vertices");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.vs_invocations		;}, root, "async.pipeline_stat.vs_invocations");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_stat.ps_invocations		;}, root, "async.pipeline_stat.ps_invocations");
		
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.internal_stat.backend_input_pixels	;}, root, "async.internal_stat.backend_input_pixels");

	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.gather_vtx			;}, root, "async.pipeline_prof.gather_vtx");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.vtx_proc				;}, root, "async.pipeline_prof.vtx_proc");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.clipping				;}, root, "async.pipeline_prof.clipping");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.compact_clip			;}, root, "async.pipeline_prof.compact_clip");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.vp_trans				;}, root, "async.pipeline_prof.vp_trans");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.tri_dispatch			;}, root, "async.pipeline_prof.tri_dispatch");
	reduce_and_output<uint64_t>(frame_profs_.begin(), frame_profs_.end(), [](frame_data const& v) { return v.pipeline_prof.ras					;}, root, "async.pipeline_prof.ras");

	write_json(file_name, root);
}

size_t benchmark::total_frames() const
{
	return frame_profs_.size();
}

END_NS_SALVIAU();
