#pragma once

#include <eflib/platform/stdint.h>
#include <eflib/utility/enum.h>
#include <functional>

enum class storage_mode : uint32_t {
  none,
  constant,
  register_id,
  stack_based_address
};
