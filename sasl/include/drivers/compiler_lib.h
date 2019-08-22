#pragma once

#include <sasl/include/drivers/drivers_forward.h>

#include <memory>

BEGIN_NS_SASL_DRIVERS();
class compiler;
std::shared_ptr<compiler> create_compiler();
END_NS_SASL_DRIVERS();
