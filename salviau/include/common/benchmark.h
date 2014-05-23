#pragma once

#include <salviau/include/salviau_forward.h>

#include <salviax/include/resource/texture/tex_io.h>
#include <salviar/include/async_object.h>
#include <salviar/include/surface.h>
#include <salviar/include/renderer.h>
#include <eflib/include/diagnostics/profiler.h>

#include <string>
#include <vector>

BEGIN_NS_SALVIAU();

struct frame_data
{
	salviar::pipeline_statistics pipeline_stat;
	salviar::internal_statistics internal_stat;
	salviar::pipeline_profiles   pipeline_prof;
};

class SALVIAU_API benchmark
{
public:
	benchmark(std::string const& benchmark_name);
	virtual ~benchmark();

	virtual void initialize() = 0;
	
	virtual void run() = 0;

	void begin_bench();
	void end_bench();

	void begin_frame();
	void end_frame();
	
	void profiling(std::string const& stage_name, std::function<void()> const& fn);

	void save_results(std::string const& file_name);

	size_t total_frames() const;
	
protected:
	/** Properties @{ */
	std::string					benchmark_name_;
	salviar::renderer_ptr		renderer_;
	eflib::profiler				prof_;

	salviar::async_object_ptr   pipeline_stat_obj_;
    salviar::async_object_ptr   internal_stat_obj_;
	salviar::async_object_ptr	pipeline_prof_obj_;

	std::vector<frame_data>		frame_profs_;
};

END_NS_SALVIAU();
