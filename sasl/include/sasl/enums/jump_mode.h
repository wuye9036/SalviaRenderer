#pragma once

#include <eflib/platform/stdint.h>

enum class jump_mode : uint32_t {
  none,
  e_break,
  e_continue,
  e_return
};
