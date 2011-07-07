#ifndef SALVIAU_COMMON_APPLICATION_H
#define SALVIAU_COMMON_APPLICATION_H

BEGIN_NS_SALVIAU();

class window;

class application{
public:
	virtual int run() = 0;
protected:
	window* main_wnd;
};

END_NS_SALVIAU();

#endif