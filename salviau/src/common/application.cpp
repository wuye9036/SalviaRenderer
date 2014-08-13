#include <salviau/include/common/application.h>
#include <salviau/include/common/window.h>

#include <cassert>

BEGIN_NS_SALVIAU();

quick_app::quick_app( application* impl ): impl(impl)
{
	main_wnd = impl->main_window();
	assert( main_wnd );
	if( main_wnd )
	{
		main_wnd->set_create_handler( boost::bind( &quick_app::on_create, this ) );
		main_wnd->set_idle_handler( boost::bind( &quick_app::on_idle, this ) );
		main_wnd->set_draw_handler( boost::bind( &quick_app::on_draw, this ) );
	}
}

quick_app::~quick_app()
{
	delete impl;
}

int quick_app::run()
{
	if( !impl )
	{
		return -1;
	}
	return impl->run();
}

void quick_app::on_create() {}
void quick_app::on_draw() {}
void quick_app::on_idle() {}

END_NS_SALVIAU();