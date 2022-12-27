#include <salviau/include/common/sample_app.h>

#if defined(EFLIB_WINDOWS)
#include <salviau/include/win/win_gui.h>
#endif
#include <salviau/include/common/window.h>
#include <salviax/include/resource/texture/tex_io.h>

#include <salvia/resource/texture.h>
#include <salviar/include/async_renderer.h>
#include <salviar/include/sync_renderer.h>

#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

#if defined(EFLIB_WINDOWS)
#include <Windows.h>
#endif

namespace po = boost::program_options;
using namespace std;
using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace boost::property_tree;

BEGIN_NS_SALVIAU();

std::string compiler_name() {
  std::stringstream s;
#if defined(BOOST_COMP_MSVC)
  s << "msvc_" << BOOST_COMP_MSVC;
#elif defined(BOOST_COMP_GUNC)
  s << "gcc_" << BOOST_COMP_MSVC;
#elif defined(BOOST_COMP_CLANG)
  s << "clang_" << BOOST_COMP_MSVC;
#elif defined(BOOST_COMP_INTEL)
  s << "intel_" << BOOST_COMP_MSVC;
#else
#error "Unknown compiler."
#endif
  return s.str();
}

sample_app::sample_app(std::string const &app_name) : data_(new sample_app_data) {
  data_->benchmark_name = app_name;
  data_->mode = app_modes::unknown;
  data_->quiting = false;
  data_->runnable = true;
  data_->frame_count = 0;
  data_->total_elapsed_sec = 0.0;
  data_->elapsed_sec = 0.0;
  data_->gui = nullptr;
  data_->is_sync_renderer = std::optional<bool>{};
  data_->frames_in_second = 0;
  data_->quit_cond = quit_conditions::user_defined;
  data_->quit_cond_data = 0;
}

sample_app::~sample_app() {
  if (data_->gui) {
    delete data_->gui;
  }
}

void sample_app::init(int argc, std::_tchar const **argv) {
  init_params(argc, argv);

  if (data_->runnable) {
    if (data_->mode == app_modes::benchmark) {
#if defined(EFLIB_WINDOWS)
      HANDLE process_handle = GetCurrentProcess();
      SetPriorityClass(process_handle, HIGH_PRIORITY_CLASS);
#endif

      data_->prof.start(data_->benchmark_name, 0);
    }

    on_init();
  }
}

void sample_app::init_params(int argc, std::_tchar const **argv) {
  cout << "Initialize with parameters ..." << endl;

  po::options_description opdesc("Sample parameters");
  opdesc.add_options()("help", "show help message")(
      "mode,m", po::value<string>()->default_value("r"),
      "mode should be one of: r - replay, i - interactive, b - benchmark or t - test.")(
      "width,w", po::value<int>()->default_value(512),
      "width of screen or back buffer. should be in 1 - 8192")(
      "height,h", po::value<int>()->default_value(512),
      "height of screen or back buffer. should be in 1 - 8192");

  auto parsed = po::parse_command_line(argc, argv, opdesc);

  po::variables_map var_map;
  po::store(parsed, var_map);

  if (var_map.count("help") || var_map.count("mode") == 0) {
    cout << opdesc;
    data_->runnable = false;
    return;
  }

  auto mode_str = var_map["mode"].as<string>();
  if (mode_str.length() != 1) {
    cout << "Error: -m or --mode used incorrect. Please see help." << endl;
    data_->runnable = false;
    return;
  }

  switch (mode_str[0]) {
  case 'r':
    data_->mode = app_modes::replay;
    break;
  case 'i':
    data_->mode = app_modes::interactive;
    break;
  case 'b':
    data_->mode = app_modes::benchmark;
    break;
  case 't':
    data_->mode = app_modes::test;
    break;
  default:
    cout << "Error: argument for -m or --mode is incorrect. Please see help." << endl;
    data_->runnable = false;
    break;
  }

  cout << "Execution mode is " << mode_str << endl;

  auto screen_width = var_map["width"].as<int>();
  auto screen_height = var_map["height"].as<int>();

  if (screen_width < 1 || 8192 < screen_width) {
    cout << "Error: screen width must be in range (0, 8192]." << endl;
    data_->runnable = false;
  } else {
    data_->screen_width = static_cast<uint32_t>(screen_width);
  }

  if (screen_height < 1 || 8192 < screen_height) {
    cout << "Error: screen height must be in range (0, 8192]." << endl;
    data_->runnable = false;
  } else {
    data_->screen_height = static_cast<uint32_t>(screen_height);
  }

  data_->screen_aspect_ratio = static_cast<float>(data_->screen_width) / data_->screen_height;

  data_->screen_vp.x = data_->screen_vp.y = 0;
  data_->screen_vp.w = static_cast<float>(screen_width);
  data_->screen_vp.h = static_cast<float>(screen_height);
  data_->screen_vp.minz = 0.0f;
  data_->screen_vp.maxz = 1.0f;

  cout << "Screen resolution: " << screen_width << "x" << screen_height << endl;

  data_->second_timer.restart();
}

void sample_app::create_devices_and_targets(size_t width, size_t height, size_t sample_count,
                                            salviar::pixel_format color_fmt,
                                            salviar::pixel_format ds_format) {
  if (data_->mode == app_modes::unknown) {
    return;
  }

  switch (data_->mode) {
  case app_modes::interactive:
  case app_modes::replay:
#if defined(EFLIB_WINDOWS)
    cout << "Create GUI ..." << endl;
    data_->gui = create_win_gui();
    break;
#else
    return;
#endif
  };

  void *wnd_handle = nullptr;
  if (data_->gui) {
    data_->gui->create_window(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    wnd_handle = data_->gui->main_window()->view_handle_as_void();
    if (!wnd_handle) {
      cout << "Error: window creation is failed." << endl;
      data_->runnable = false;
      return;
    }

    data_->gui->main_window()->set_title(data_->benchmark_name);
  }

  cout << "Create devices and targets ..." << endl;

  renderer_parameters rparams = {width, height, sample_count, color_fmt, wnd_handle};

  renderer_types rtype = salviax::renderer_none;
  swap_chain_types sc_type = data_->gui ? salviax::swap_chain_default : salviax::swap_chain_none;

  if (data_->is_sync_renderer.has_value()) {
    rtype = data_->is_sync_renderer.value() ? salviax::renderer_sync : salviax::renderer_async;
  } else {
    switch (data_->mode) {
    case app_modes::interactive:
    case app_modes::replay:
    case app_modes::test:
      rtype = salviax::renderer_async;
      break;
    case app_modes::benchmark:
      rtype = salviax::renderer_sync;
      break;
    }
  }

  salviax_create_swap_chain_and_renderer(data_->swap_chain, data_->renderer, &rparams, rtype,
                                         sc_type);

  if (!data_->renderer) {
    return;
  }

  if (data_->swap_chain) {
    data_->color_target = data_->swap_chain->get_surface();
  } else {
    data_->color_target =
        data_->renderer->create_tex2d(width, height, sample_count, color_fmt)->subresource(0);
  }

  if (sample_count > 1) {
    data_->resolved_color_target =
        data_->renderer->create_tex2d(width, height, 1, color_fmt)->subresource(0);
  } else {
    data_->resolved_color_target = data_->color_target;
  }

  if (ds_format != pixel_format_invalid) {
    data_->ds_target =
        data_->renderer->create_tex2d(width, height, sample_count, ds_format)->subresource(0);
  }

  data_->renderer->set_render_targets(1, &data_->color_target, data_->ds_target);

  if (data_->gui) {
    data_->gui->main_window()->set_idle_handler([this]() { this->on_gui_idle(); });
    data_->gui->main_window()->set_draw_handler([this]() { this->on_gui_draw(); });
  }

  if (data_->mode == app_modes::benchmark) {
    data_->pipeline_stat_obj = data_->renderer->create_query(async_object_ids::pipeline_statistics);
    data_->internal_stat_obj = data_->renderer->create_query(async_object_ids::internal_statistics);
    data_->pipeline_prof_obj = data_->renderer->create_query(async_object_ids::pipeline_profiles);
  }
}

void sample_app::draw_frame() {
  data_->elapsed_sec = data_->frame_timer.elapsed();
  data_->total_elapsed_sec += data_->elapsed_sec;
  data_->frame_timer.restart();

  switch (data_->quit_cond) {
  case quit_conditions::frame_limits:
    if (data_->frame_count >= data_->quit_cond_data) {
      quit();
    }
    break;
  case quit_conditions::time_out:
    if (data_->total_elapsed_sec > data_->quit_cond_data / 1000.0) {
      quit();
    }
    break;
  }

  if (data_->quiting) {
    return;
  }

  if (data_->mode == app_modes::benchmark) {
    data_->renderer->begin(data_->pipeline_stat_obj);
    data_->renderer->begin(data_->internal_stat_obj);
    data_->renderer->begin(data_->pipeline_prof_obj);
  }

  on_frame();

  if (data_->mode == app_modes::benchmark) {
    data_->renderer->end(data_->pipeline_stat_obj);
    data_->renderer->end(data_->internal_stat_obj);
    data_->renderer->end(data_->pipeline_prof_obj);
  }

  if (data_->quiting) {
    return;
  }

  if (data_->swap_chain) {
    data_->swap_chain->present();
  }

  if (data_->mode == app_modes::benchmark) {
    frame_data frame_prof;

    data_->renderer->get_data(data_->pipeline_stat_obj, &frame_prof.pipeline_stat, false);
    data_->renderer->get_data(data_->internal_stat_obj, &frame_prof.internal_stat, false);
    data_->renderer->get_data(data_->pipeline_prof_obj, &frame_prof.pipeline_prof, false);

    data_->frame_profs.push_back(frame_prof);
  }

  ++data_->frame_count;
  ++data_->frames_in_second;

  double current_time = data_->second_timer.elapsed();
  double frame_elapsed = data_->frame_timer.elapsed();
  if ((current_time >= 1.0) || (1.0 - current_time < frame_elapsed)) {
    cout.precision(3);
    cout << "Frame: #" << data_->frame_count << " | FPS: " << data_->frames_in_second / current_time
         << endl;
    std::cout.unsetf(std::ios::floatfield);

    data_->second_timer.restart();
    data_->frames_in_second = 0;
  }

  switch (data_->mode) {
  case app_modes::test:
    data_->renderer->flush();
    if (data_->color_target != data_->resolved_color_target) {
      data_->color_target->resolve(*data_->resolved_color_target);
    }
    save_frame(data_->resolved_color_target);
    break;
  }
}

void sample_app::on_gui_idle() { draw_frame(); }

void sample_app::on_gui_draw() {}

void sample_app::run() {
  if (!data_->runnable) {
    return;
  }

  cout << "Start running ..." << endl;

  data_->frame_count = 0;

  if (data_->gui) {
    data_->gui->run();
  } else {
    while (!data_->quiting) {
      draw_frame();
    }
  }

  if (data_->mode == app_modes::benchmark) {
    data_->prof.end(data_->benchmark_name);
    save_profiling_result();
    print_profiler(&data_->prof, 3);
  }

  cout << "Running done." << endl;
}

void sample_app::quit_at_frame(uint32_t frame_cnt) {
  data_->quit_cond = quit_conditions::frame_limits;
  data_->quit_cond_data = frame_cnt;
}

void sample_app::quit_if_time_out(uint32_t milli_sec) {
  data_->quit_cond = quit_conditions::time_out;
  data_->quit_cond_data = milli_sec;
}

void sample_app::quit() {
  cout << "Exiting ..." << endl;
  data_->quiting = true;
}

// Utilities
void sample_app::profiling(std::string const &stage_name, std::function<void()> const &fn) {
  if (data_->mode == app_modes::benchmark) {
    eflib::profiling_scope pscope(&data_->prof, stage_name, 0);
    fn();
  } else {
    fn();
  }
}

void sample_app::save_frame(salviar::surface_ptr const &surf) {
  stringstream ss;
  ss << data_->benchmark_name << "_" << data_->frame_count - 1 << ".png";
  salviax::resource::save_surface(data_->renderer.get(), surf, eflib::to_tstring(ss.str()),
                                  pixel_format_color_bgra8);
}

template <typename T> void min_max(T &in_out_min, T &in_out_max, T v) {
  if (v < in_out_min) {
    in_out_min = v;
  } else if (v > in_out_max) {
    in_out_max = v;
  }
}

template <typename ValueT, typename IterT, typename TransformT>
void reduce_and_output(IterT beg, IterT end, TransformT trans, ptree &parent,
                       std::string const &path) {
  ValueT minv;
  ValueT maxv;
  ValueT total = 0;
  ValueT avg = 0;

  if (beg == end) {
    minv = maxv = total = avg = 0;
  } else {
    auto it = beg;
    minv = maxv = total = trans(*it);
    ++it;

    size_t count = 1;
    for (; it != end; ++it) {
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

static int const OUTPUT_PROFILER_LEVEL = 3;
void sample_app::save_profiling_result() {
  data_->prof.merge_items();

  stringstream ss;
  ss << data_->benchmark_name << "_"
     << "Profiling.json";

  auto root = make_ptree(&data_->prof, OUTPUT_PROFILER_LEVEL);

  // Metadata
  root.put("compiler", compiler_name());

  // Statistic
  root.put("frames", data_->frame_count);

  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.cinvocations; }, root,
      "async.pipeline_stat.cinvocations");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.cprimitives; }, root,
      "async.pipeline_stat.cprimitives");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.ia_primitives; }, root,
      "async.pipeline_stat.ia_primitives");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.ia_vertices; }, root,
      "async.pipeline_stat.ia_vertices");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.vs_invocations; }, root,
      "async.pipeline_stat.vs_invocations");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_stat.ps_invocations; }, root,
      "async.pipeline_stat.ps_invocations");

  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.internal_stat.backend_input_pixels; }, root,
      "async.internal_stat.backend_input_pixels");

  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.gather_vtx; }, root,
      "async.pipeline_prof.gather_vtx");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.vtx_proc; }, root,
      "async.pipeline_prof.vtx_proc");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.clipping; }, root,
      "async.pipeline_prof.clipping");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.compact_clip; }, root,
      "async.pipeline_prof.compact_clip");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.vp_trans; }, root,
      "async.pipeline_prof.vp_trans");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.tri_dispatch; }, root,
      "async.pipeline_prof.tri_dispatch");
  reduce_and_output<uint64_t>(
      data_->frame_profs.begin(), data_->frame_profs.end(),
      [](frame_data const &v) { return v.pipeline_prof.ras; }, root, "async.pipeline_prof.ras");

  write_json(ss.str(), root);
}

END_NS_SALVIAU();
