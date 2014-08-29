#include <salviau/include/common/sample_app.h>

#if defined(EFLIB_WINDOWS)
#	include <salviau/include/win/win_application.h>
#endif
#include <salviau/include/common/window.h>
#include <salviax/include/resource/texture/tex_io.h>

#include <salviar/include/async_renderer.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/texture.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <eflib/include/platform/boost_end.h>

#if defined(EFLIB_WINDOWS)
#	include <Windows.h>
#endif

namespace po = boost::program_options;
using namespace std;
using namespace eflib;
using namespace salviar;
using namespace salviax;
using namespace boost::property_tree;

BEGIN_NS_SALVIAU();

sample_app::sample_app(std::string const& app_name):
	data_(new sample_app_data)
{
	data_->benchmark_name = app_name;
	data_->mode = app_modes::unknown;
	data_->quiting = false;
	data_->runnable = true;
	data_->frame_count = 0;
	data_->elapsed_sec = 0.0;
}

sample_app::~sample_app()
{
}

void sample_app::init(int argc, std::_tchar const** argv)
{
	init_params(argc, argv);

	if(data_->runnable)
	{
		if(data_->mode == app_modes::benchmark)
		{
		#if defined(EFLIB_WINDOWS)
			HANDLE process_handle = GetCurrentProcess();
			SetPriorityClass(process_handle, HIGH_PRIORITY_CLASS);
		#endif

			data_->prof.start(data_->benchmark_name, 0); 
		}

		on_init();
	}
}

void sample_app::init_params(int argc, std::_tchar const** argv)
{
	po::options_description opdesc("Sample parameters");
	opdesc.add_options()
		("help,h", "show help message")
		("mode,m",
			po::value<string>()->default_value("r"),
			"mode should be one of: r - replay, i - interactive, b - benchmark or t - test.")
		;
	
	auto parsed = po::parse_command_line(argc, argv, opdesc);
	
	po::variables_map var_map;
	po::store(parsed, var_map);

	if ( var_map.count("help") || var_map.count("mode") == 0)
	{
		cout << opdesc;
		data_->runnable = false;
		return;
	}

	auto mode_str = var_map["mode"].as<string>();
	if (mode_str.length() != 1)
	{
		cout << "Error: -m or --mode used incorrect. Please see help." << endl;
		data_->runnable = false;
		return;
	}

	switch(mode_str[0])
	{
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
}

void sample_app::create_devices_and_targets(
	size_t width, size_t height,
	size_t sample_count,
	salviar::pixel_format color_fmt, salviar::pixel_format ds_format
	)
{
	if(data_->mode == app_modes::unknown)
	{
		return;
	}

	switch(data_->mode)
	{
	case app_modes::interactive:
	case app_modes::replay:
#if defined(EFLIB_WINDOWS)
		data_->gui = create_win_gui();
		break;
#else
		return;
#endif
	};

	void* wnd_handle = nullptr;
	if (data_->gui)
	{
		data_->gui->create_window();
		wnd_handle = data_->gui->main_window()->view_handle_as_void();
		if(!wnd_handle)
		{
			cout << "Error: window creation is failed." << endl;
			data_->runnable = false;
			return;
		}
	}

	renderer_parameters rparams = {
		width, height, sample_count, color_fmt, wnd_handle
	};

	renderer_types rtype = salviax::renderer_none;

	if (data_->is_sync_renderer)
	{
		rtype = salviax::renderer_sync;
	}
	else if (!data_->is_sync_renderer)
	{
		rtype = salviax::renderer_async;
	}
	else
	{
		switch(data_->mode)
		{
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

	salviax_create_swap_chain_and_renderer(data_->swap_chain, data_->renderer, &rparams, rtype);

	if(!data_->renderer)
	{
		return;
	}

	if(data_->swap_chain)
	{
		data_->color_target = data_->swap_chain->get_surface();
	}
	else
	{
		data_->color_target =
			data_
			->renderer->create_tex2d(width, height, sample_count, color_fmt)
			->subresource(0);
	}

	if(sample_count > 1)
	{
		data_->resolved_color_target = 
			data_
			->renderer->create_tex2d(width, height, 1, color_fmt)
			->subresource(0);
	}
	else
	{
		data_->resolved_color_target = data_->color_target;
	}

	if(ds_format != pixel_format_invalid)
	{
		data_->ds_target =
			data_
			->renderer->create_tex2d(width, height, sample_count, ds_format)
			->subresource(0);
	}

	data_->renderer->set_render_targets(1, &data_->color_target, data_->ds_target);

	if(data_->gui)
	{
		data_->gui->main_window()->set_idle_handler( [this]() { this->on_gui_idle(); } );
		data_->gui->main_window()->set_draw_handler( [this]() { this->on_gui_draw(); } );
	}
}

void sample_app::draw_frame()
{
	if(data_->mode == app_modes::benchmark)
	{
		data_->renderer->begin(data_->pipeline_stat_obj);
		data_->renderer->begin(data_->internal_stat_obj);
		data_->renderer->begin(data_->pipeline_prof_obj);
	}

	data_->elapsed_sec = data_->t.elapsed();
	data_->t.restart();
	on_frame();
	if(data_->mode == app_modes::benchmark)
	{
		data_->renderer->end(data_->pipeline_stat_obj);
		data_->renderer->end(data_->internal_stat_obj);
		data_->renderer->end(data_->pipeline_prof_obj);
	}

	if(data_->quiting)
	{
		return;
	}

	++data_->frame_count;

	if(data_->mode == app_modes::benchmark)
	{
		frame_data frame_prof;

		data_->renderer->get_data(data_->pipeline_stat_obj, &frame_prof.pipeline_stat, false);
		data_->renderer->get_data(data_->internal_stat_obj, &frame_prof.internal_stat, false); 
		data_->renderer->get_data(data_->pipeline_prof_obj, &frame_prof.pipeline_prof, false);

		data_->frame_profs.push_back(frame_prof);
	}

	if(data_->swap_chain)
	{
		data_->swap_chain->present();
	}

	switch(data_->mode)
	{
	case app_modes::test:
		if(data_->color_target != data_->resolved_color_target)
		{
			data_->color_target->resolve(*data_->resolved_color_target);
		}
		save_frame(data_->resolved_color_target);
		break;
	}
}

void sample_app::on_gui_idle()
{
	draw_frame();
}

void sample_app::on_gui_draw()
{
}

void sample_app::run()
{
	if(!data_->runnable)
	{
		return;
	}

	data_->frame_count = 0;

	if(data_->gui)
	{
		data_->gui->run();
	}
	else
	{	
		while(!data_->quiting)
		{
			draw_frame();
		}
	}

	if(data_->mode == app_modes::benchmark)
	{
		data_->prof.end(data_->benchmark_name);
		save_profiling_result();
	}
}
	
void sample_app::quit()
{
	data_->quiting = true;
}

// Utilities
void sample_app::profiling(std::string const& stage_name, std::function<void()> const& fn)
{
	if(data_->mode == app_modes::benchmark)
	{
		data_->prof.start(stage_name, 0);
		fn();
		data_->prof.end(stage_name);
	}
	else
	{
		fn();
	}
}

void sample_app::save_frame(salviar::surface_ptr const& surf)
{
	stringstream ss;
	ss << data_->benchmark_name << "_" << data_->frame_count - 1 << ".png";
	salviax::resource::save_surface(data_->renderer.get(), surf, eflib::to_tstring( ss.str() ), pixel_format_color_bgra8);
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

static int const OUTPUT_PROFILER_LEVEL = 3;
void sample_app::save_profiling_result()
{
	data_->prof.merge_items();

	stringstream ss;
	ss << data_->benchmark_name << "_" << "profiling.json";

	auto root = make_ptree(&data_->prof, OUTPUT_PROFILER_LEVEL);

	// Statistic
	root.put("frames", data_->frame_count);

	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.cinvocations			;}, root, "async.pipeline_stat.cinvocations");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.cprimitives			;}, root, "async.pipeline_stat.cprimitives");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.ia_primitives		;}, root, "async.pipeline_stat.ia_primitives");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.ia_vertices			;}, root, "async.pipeline_stat.ia_vertices");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.vs_invocations		;}, root, "async.pipeline_stat.vs_invocations");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_stat.ps_invocations		;}, root, "async.pipeline_stat.ps_invocations");
		
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.internal_stat.backend_input_pixels	;}, root, "async.internal_stat.backend_input_pixels");

	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.gather_vtx			;}, root, "async.pipeline_prof.gather_vtx");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.vtx_proc				;}, root, "async.pipeline_prof.vtx_proc");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.clipping				;}, root, "async.pipeline_prof.clipping");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.compact_clip			;}, root, "async.pipeline_prof.compact_clip");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.vp_trans				;}, root, "async.pipeline_prof.vp_trans");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.tri_dispatch			;}, root, "async.pipeline_prof.tri_dispatch");
	reduce_and_output<uint64_t>(data_->frame_profs.begin(), data_->frame_profs.end(), [](frame_data const& v) { return v.pipeline_prof.ras					;}, root, "async.pipeline_prof.ras");

	write_json(ss.str(), root);
}

END_NS_SALVIAU();
