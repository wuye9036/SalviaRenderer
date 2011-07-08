#ifndef SALVIAU_COMMON_WINDOW_H
#define SALVIAU_COMMON_WINDOW_H

#include <salviau/include/salviau_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAU();

typedef boost::function<void()> idle_handler_t;
typedef boost::function<void()> draw_handler_t;
typedef boost::function<void()> create_handler_t;

class window{
public:
	virtual void show() = 0;

	/** Event handlers
	@{ */
	virtual void set_idle_handler( idle_handler_t const& handler ) = 0;
	virtual void set_draw_handler( draw_handler_t const& handler ) = 0;
	virtual void set_create_handler( create_handler_t const& handler ) = 0;
	/** @} */
	
	virtual boost::any view_handle() = 0;

	virtual void refresh() = 0;
};

END_NS_SALVIAU();

#endif