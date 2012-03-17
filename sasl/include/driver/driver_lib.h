#ifndef SASL_DRIVER_STATIC_DRIVER_API_H
#define SASL_DRIVER_STATIC_DRIVER_API_H

#include <sasl/include/driver/driver_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_DRIVER();
class driver;
boost::shared_ptr<driver> create_driver();
END_NS_SASL_DRIVER();

#endif