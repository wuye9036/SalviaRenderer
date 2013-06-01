#pragma once

#include <salviar/include/salviar_forward.h>
#include <salviar/include/decl.h>
#include <eflib/include/math/math.h>

BEGIN_NS_SALVIAR();

class device
{
public:
	virtual ~device(){};
	virtual void present(const salviar::surface& surf) = 0;
};

END_NS_SALVIAR();
