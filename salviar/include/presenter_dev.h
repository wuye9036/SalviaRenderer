#ifndef SALVIAR_DEV_H
#define SALVIAR_DEV_H

#include "salviar/include/decl.h"

#include <eflib/include/math/math.h>
#include <salviar/include/salviar_forward.h>
BEGIN_NS_SALVIAR()


class device;
DECL_HANDLE(device, h_device)

class device
{
public:
	virtual ~device(){};
	virtual void present(const salviar::surface& surf) = 0;
};

END_NS_SALVIAR()

#endif