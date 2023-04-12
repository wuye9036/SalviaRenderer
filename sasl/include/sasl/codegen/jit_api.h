#pragma once

#include <sasl/codegen/forward.h>

#include <string_view>
#include <vector>

namespace sasl::codegen {

class codegen_context;

class jit_engine {
public:
  virtual void* get_function(std::string_view func_name) = 0;
  virtual void inject_function(void* fn, std::string_view fn_name) = 0;

protected:
  jit_engine() {}
  virtual ~jit_engine() {}

private:
  jit_engine(const jit_engine&);
  jit_engine& operator=(const jit_engine&);
};

}  // namespace sasl::codegen