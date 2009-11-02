#ifndef SOFTART_DEV_H
#define SOFTART_DEV_H

#include "softart/include/decl.h"

#include "eflib/include/math.h"

class device
{
public:
	virtual ~device(){};
	virtual void attach_framebuffer(framebuffer* pfb) = 0;
	virtual void present(const efl::rect<size_t>& src_rect, const efl::rect<size_t>& dest_rect) = 0;
};

#endif