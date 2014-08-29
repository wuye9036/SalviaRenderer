#include <salviau/include/common/sample_app.h>

#if defined(EFLIB_WINDOWS)
#	include <salviau/include/win/win_application.h>
#endif
#include <salviau/include/common/window.h>

#include <salviar/include/async_renderer.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/texture.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/program_options.hpp>
#include <eflib/include/platform/boost_end.h>

namespace po = boost::program_options;
using namespace std;
using namespace salviar;
using namespace salviax;

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
	data_->elapsed_sec = data_->t.elapsed();
	data_->t.restart();
	on_frame();
	++data_->frame_count;

	if(data_->swap_chain)
	{
		data_->swap_chain->present();
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
}
	
void sample_app::quit()
{
	data_->quiting = true;
}

// Utilities
void sample_app::profiling(std::string const& stage_name, std::function<void()> const& fn)
{
}

void sample_app::save_frame(salvia::surface_ptr const& surf)
{
}

void sample_app::save_profiling_result(std::string const& file_name)
{
}

END_NS_SALVIAU();
