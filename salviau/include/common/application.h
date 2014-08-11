#ifndef SALVIAU_COMMON_APPLICATION_H
#define SALVIAU_COMMON_APPLICATION_H

#include <salviau/include/salviau_forward.h>

BEGIN_NS_SALVIAU();

class window;

class application
{
public:
	virtual int run() = 0;
	virtual window* main_window() = 0;
};

class SALVIAU_API quick_app
{
public:
	quick_app( application* impl );
	virtual ~quick_app();

	virtual int run();

protected:
	virtual void on_create();
	virtual void on_draw();
	virtual void on_idle();

protected:
	application* impl;
	window* main_wnd;
};

END_NS_SALVIAU();

#endif