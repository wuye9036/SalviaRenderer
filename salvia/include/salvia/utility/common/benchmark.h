#pragma once

#include <salvia/utility/api_symbols.h>

#include <eflib/diagnostics/profiler.h>
#include <salvia/core/renderer.h>
#include <salvia/resource/surface.h>
#include <salvia/core/async_object.h>
#include <salvia/ext/resource/texture/tex_io.h>

#include <string>
#include <vector>

namespace salvia::utility {

struct frame_data {
  salviar::pipeline_statistics pipeline_stat;
  salviar::internal_statistics internal_stat;
  salviar::pipeline_profiles pipeline_prof;
};

class SALVIA_UTILITY_API benchmark {
public:
  benchmark(std::string const &benchmark_name);
  virtual ~benchmark();

  virtual void initialize() = 0;

  virtual void run() = 0;

  void begin_bench();
  void end_bench();

  void begin_frame();
  void end_frame();

  void profiling(std::string const &stage_name, std::function<void()> const &fn);

  void save_results(std::string const &file_name);

  size_t total_frames() const;

protected:
  /** Properties @{ */
  std::string benchmark_name_;
  salvia::core::renderer_ptr renderer_;
  eflib::profiler prof_;

  salviar::async_object_ptr pipeline_stat_obj_;
  salviar::async_object_ptr internal_stat_obj_;
  salviar::async_object_ptr pipeline_prof_obj_;

  std::vector<frame_data> frame_profs_;
};

}
