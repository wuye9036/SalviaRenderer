#pragma once

#ifndef SASL_DRIVERS_COMPILER_LIB_H
#define SASL_DRIVERS_COMPILER_LIB_H

#include <sasl/include/drivers/drivers_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_DRIVERS();
class compiler;
boost::shared_ptr<compiler> create_compiler();
END_NS_SASL_DRIVERS();

#endif