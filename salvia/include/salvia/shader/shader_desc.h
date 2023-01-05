#pragma once

#include <string>

namespace salvia::shader {

struct external_function_desc {
  external_function_desc(void *func, std::string const &func_name, bool is_raw_name)
      : func(func), func_name(func_name), is_raw_name(is_raw_name) {}
  void *func;
  std::string func_name;
  bool is_raw_name;
};

} // namespace salvia::shader