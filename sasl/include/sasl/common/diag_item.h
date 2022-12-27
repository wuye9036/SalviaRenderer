#pragma once

#include <sasl/common/token.h>

#include <fmt/format.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sasl::common {

enum class diag_levels { fatal_error, error, warning, info, debug };

struct diag_template {
  size_t uid;
  diag_levels level;
  std::string_view content;
};

class diag_item {
public:
  constexpr diag_item() noexcept = default;
  constexpr diag_item(diag_template t, std::string_view file_name, code_span span,
                      std::string resolved_diag) noexcept
      : file_name_{file_name}, span_{span}, resolved_diag_(std::move(resolved_diag)), template_{t} {
  }
  constexpr diag_item(diag_item const &) = default;
  constexpr diag_item(diag_item &&rhs) noexcept = default;
  constexpr diag_item &operator=(diag_item &&rhs) noexcept = default;
  constexpr diag_item &operator=(diag_item const &rhs) = default;

  constexpr diag_levels level() const noexcept { return template_.level; }
  constexpr size_t id() const noexcept { return template_.uid; };
  constexpr std::string_view file_name() const noexcept { return file_name_; }
  constexpr code_span span() const noexcept { return span_; }
  constexpr std::string_view str() const noexcept { return resolved_diag_; }

private:
  std::string_view file_name_;
  code_span span_;
  std::string resolved_diag_;
  diag_template template_;
};

} // namespace sasl::common
