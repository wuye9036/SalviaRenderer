#pragma once

#include <eflib/platform/config.h>

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

#include <iterator>
#include <memory>
#include <string_view>

namespace sasl::common {

struct code_pos {
  size_t line = 0, column = 0;
};

constexpr std::weak_ordering operator<=>(code_pos lhs, code_pos rhs) noexcept {
  return std::make_pair(lhs.line, lhs.column) <=> std::make_pair(rhs.line, rhs.column);
}

struct code_span {
  code_pos begin, end;
};

constexpr code_span merge(code_span const &s0, code_span const &s1) {
  return code_span{.begin = std::min(s0.begin, s1.begin), .end = std::max(s0.end, s1.end)};
}

constexpr code_span inline_code_span(size_t line, size_t col_beg, size_t col_end) {
  return code_span{.begin = {line, col_beg}, .end = {line, col_end}};
}

namespace token_storage_policy {
struct shared {};
struct unique {};
} // namespace token_storage_policy

template <typename StoragePolicy> class [[nodiscard]] token_base {
private:
  static constexpr bool is_shared_storage =
      std::is_same_v<StoragePolicy, token_storage_policy::shared>;

  struct token_data {
    size_t id = 0;
    std::string_view lit;
    code_span span;
    std::string_view file_name;
    bool end_of_file = false;
  };

  using data_ptr = std::conditional_t<is_shared_storage, boost::local_shared_ptr<token_data>,
                                      std::unique_ptr<token_data>>;

  data_ptr data_;

  template <typename... Args> static data_ptr make_data(Args &&...args) {
    if constexpr (is_shared_storage) {
      return boost::make_local_shared<token_data>(std::forward<Args>(args)...);
    } else {
      return std::make_unique<token_data>(std::forward<Args>(args)...);
    }
  }

  template <typename SP, typename = std::enable_if_t<is_shared_storage>>
  token_base &assign(token_base<SP> const &rhs) noexcept {
    data_ = rhs.data_;
    return *this;
  }

public:
  EF_CONSTEXPR23 token_base() {}

  // Move constructs applied on shared and unique
  EF_CONSTEXPR23 token_base(token_base &&rhs) noexcept = default;
  EF_CONSTEXPR23 token_base &operator=(token_base &&rhs) noexcept = default;

  EF_CONSTEXPR23 token_base(token_base const &rhs) noexcept { assign(rhs); }
  EF_CONSTEXPR23 token_base &operator=(token_base const &rhs) noexcept { return assign(rhs); }

  EF_CONSTEXPR23 token_base clone() const {
    auto ret = token_base(make_data(*data_));
    return ret;
  }

  EF_CONSTEXPR23 static token_base uninitialized() noexcept { return token_base{}; }
  EF_CONSTEXPR23 static token_base make_empty() { return token_base(make_data()); }

  EF_CONSTEXPR23 static token_base make(std::string_view lit) {
    return make(0, lit, 0, 0, std::string_view{});
  }
  EF_CONSTEXPR23 static token_base make(size_t id, std::string_view lit, size_t line, size_t col,
                                        std::string_view fname, bool end_of_file = false) {
    auto span = inline_code_span(line, col, col);
    size_t cur_line = line;
    size_t cur_col = col;
    for (auto ch : lit) {
      if (ch == '\n') {
        ++cur_line;
        cur_col = 1;
      } else {
        ++cur_col;
      }
    }
    span.end = {cur_line, cur_col};

    auto data = make_data(token_data{
        .id = id, .lit = lit, .span = span, .file_name = fname, .end_of_file = end_of_file});
    return token_base(std::move(data));
  }

  [[nodiscard]] EF_CONSTEXPR23 bool is_uninitialized() const noexcept { return !data_; }

  [[nodiscard]] EF_CONSTEXPR23 bool is_valid() const noexcept {
    return !is_uninitialized() && !lit().empty();
  }

  [[nodiscard]] EF_CONSTEXPR23 size_t id() const noexcept { return data_->id; }
  [[nodiscard]] EF_CONSTEXPR23 std::string_view lit() const noexcept { return data_->lit; }
  [[nodiscard]] EF_CONSTEXPR23 code_span span() const noexcept { return data_->span; }
  [[nodiscard]] EF_CONSTEXPR23 std::string_view file_name() const noexcept {
    return data_->file_name;
  }
  [[nodiscard]] EF_CONSTEXPR23 bool end_of_file() const { return data_->end_of_file; }

private:
  EF_CONSTEXPR23 explicit token_base(data_ptr data) noexcept : data_{std::move(data)} {}
};

using unique_token = token_base<token_storage_policy::unique>;
using shared_token = token_base<token_storage_policy::shared>;
using token = shared_token;

} // namespace sasl::common
