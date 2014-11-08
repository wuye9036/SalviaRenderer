#ifndef SALVIAU_COMMON_APPLICATION_H
#define SALVIAU_COMMON_APPLICATION_H

#include <salviau/include/salviau_forward.h>

BEGIN_NS_SALVIAU();

class window;

class gui
{
public:
	virtual int     create_window() = 0;
	virtual int     run() = 0;
	virtual window* main_window() = 0;
};

END_NS_SALVIAU();

#endif