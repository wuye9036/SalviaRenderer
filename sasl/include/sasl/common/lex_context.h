#pragma once

#include <string_view>

namespace sasl::common {

class lex_context {
public:
  virtual std::string_view file_name() const = 0;
  virtual size_t column() const = 0;
  virtual size_t line() const = 0;
  virtual void update_position(std::string_view lit) = 0;
};

class code_source {
public:
  virtual bool failed() = 0;
  virtual bool eof() = 0;
  virtual std::string_view next() = 0;
  virtual std::string_view error() = 0;
};

} // namespace sasl::common
