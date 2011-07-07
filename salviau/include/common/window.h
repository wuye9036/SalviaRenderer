#ifndef SALVIAU_COMMON_WINDOW_H
#define SALVIAU_COMMON_WINDOW_H

#include <salviau/include/salviau_forward.h>

BEGIN_NS_SALVIAU();

typedef boost::function<void()> idle_handler_t;
typedef boost::function<void()> draw_handler_t;

class window{
public:
	virtual void show() = 0;
	
	virtual void set_idle_handler( idle_handler_t const& handler ) = 0;
	virtual void set_draw_handler( draw_handler_t const& handler ) = 0;
	
	virtual void refresh();
};

END_NS_SALVIAU();

#endif