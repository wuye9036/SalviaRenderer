#pragma once

#ifndef SALVIAU_COMMON_PRESENTER_UTILITY_H
#define SALVIAU_COMMON_PRESENTER_UTILITY_H

#include <salviau/include/salviau_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(device);
}

BEGIN_NS_SALVIAU();

SALVIAU_API salviar::device_ptr create_default_presenter(void* param);

END_NS_SALVIAU();

#endif