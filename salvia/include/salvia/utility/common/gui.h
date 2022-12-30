#pragma once

#include <eflib/platform/stdint.h>
#include <salvia/utility/api_symbols.h>

namespace salvia::utility {

class window;

class gui {
public:
  virtual int create_window(uint32_t width, uint32_t height) = 0;
  virtual int run() = 0;
  virtual window *main_window() = 0;
  virtual ~gui() = default;
};

}
