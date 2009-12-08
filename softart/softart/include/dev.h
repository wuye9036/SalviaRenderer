#ifndef SOFTART_DEV_H
#define SOFTART_DEV_H

#include "softart/include/decl.h"

#include "eflib/include/math.h"

class device;
DECL_HANDLE(device, h_device)

class device
{
public:
	virtual ~device(){};
	virtual void attach_framebuffer(framebuffer* pfb) = 0;
	virtual void present() = 0;
};

#endif