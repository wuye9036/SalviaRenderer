#pragma once

#include <salviau/include/salviau_forward.h>

#include <salviax/include/resource/texture/tex_io.h>
#include <salviar/include/async_object.h>
#include <salviar/include/surface.h>
#include <salviar/include/renderer.h>
#include <eflib/include/diagnostics/profiler.h>

#include <string>
#include <vector>

namespace salvia
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
}

BEGIN_NS_SALVIAU();

enum class app_modes
{
	benchmark,		// Run as benchmark. It will generate some benchmark results.
	test,			// Run as regression test. It will generate final frames as image file for test.
	interactive,	// Interactive mode.
	replay			// Play mode.
};

struct frame_data
{
	salviar::pipeline_statistics pipeline_stat;
	salviar::internal_statistics internal_stat;
	salviar::pipeline_profiles   pipeline_prof;
};
#if 0
class SALVIAU_API sample_app
{
public:
	sample_app(std::string const& app_name, std::string const& options);
	virtual ~sample_app();

	virtual void run();
	
protected:
	size_t total_frames() const;
	void   exit();

	// Utilities
	virtual void profiling				(std::string const& stage_name, std::function<void()> const& fn);
	virtual void save_frame				(salvia::surface_ptr const& surf);
	virtual void save_profiling_result	(std::string const& file_name);

	// Events
	virtual void on_create() = 0;
	virtual void on_frame(size_t i_frame, size_t elapsed_ms) = 0;
	
	std::string					benchmark_name_;
	app_modes					mode_;
	std::string					param_;
	salviar::renderer_ptr		renderer_;
	eflib::profiler				prof_;

	salviar::async_object_ptr   pipeline_stat_obj_;
    salviar::async_object_ptr   internal_stat_obj_;
	salviar::async_object_ptr	pipeline_prof_obj_;

	std::vector<frame_data>		frame_profs_;

private:
	// timer	tm_;
	size_t	frame_;
	bool	exiting_;

};
#endif
END_NS_SALVIAU();
