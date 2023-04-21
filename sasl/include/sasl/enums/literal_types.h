#pragma once

#include <eflib/platform/stdint.h>

enum class literal_types : uint32_t {
  none = UINT32_C(1),
  boolean,
  integer,
  real,
  string,
  character
};
