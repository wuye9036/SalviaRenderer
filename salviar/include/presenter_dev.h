#ifndef SOFTART_DEV_H
#define SOFTART_DEV_H

#include "salviar/include/decl.h"

#include <eflib/include/math/math.h>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


class device;
DECL_HANDLE(device, h_device)

class device
{
public:
	virtual ~device(){};
	virtual void present(const softart::surface& surf) = 0;
};

END_NS_SOFTART()

#endif