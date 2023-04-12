#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace salvia::shader {

class shader_code;

typedef std::function<std::string(std::string const&)> include_hook_t;

class compiler {
public:
  void defines(std::map<std::string, std::string> const& defs);
  std::shared_ptr<shader_code> compile(std::string const& code);
};

extern "C" {
std::shared_ptr<compiler> salvia_create_compiler();
}

}  // namespace salvia::shader

#endif