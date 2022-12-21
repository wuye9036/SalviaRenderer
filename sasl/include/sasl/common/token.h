#pragma once

#include <eflib/platform/config.h>

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

template <typename StoragePolicy> class token_base {
private:
  template <typename T> struct delete_shared {
    constexpr delete_shared() noexcept = default;
    void operator()(T *ptr) const {
      --(ptr->ref_count);
      if (ptr->ref_count == 0) {
        delete ptr;
      }
    }
  };

  static constexpr bool is_shared_storage =
      std::is_same_v<StoragePolicy, token_storage_policy::shared>;

  template <bool Shared> struct ref_counter {};

  template <> struct ref_counter<true> {
    size_t ref_count = 1;
  };

  struct token_data : public ref_counter<is_shared_storage> {
    size_t id = 0;
    std::string_view lit;
    code_span span;
    std::string_view file_name;
    bool end_of_file = false;
  };

  using deleter = std::conditional_t<is_shared_storage, delete_shared<token_data>,
                                     std::default_delete<token_data>>;

  using data_ptr = std::unique_ptr<token_data, deleter>;

  data_ptr data_;

  template <typename... Args> static data_ptr make_data(Args &&...args) {
    if constexpr (is_shared_storage) {
      return data_ptr(new token_data(std::forward<Args>(args)...));
    } else {
      return std::make_unique<token_data>(std::forward<Args>(args)...);
    }
  }

  template <typename SP, typename = std::enable_if_t<is_shared_storage>>
  token_base &assign(token_base<SP> const &rhs) noexcept {
    data_.reset(rhs.data_.get());
    ++(data_->ref_count);
    return *this;
  }

public:
  // Move constructs applied on shared and unique
  EF_CONSTEXPR23 token_base(token_base &&rhs) noexcept = default;
  EF_CONSTEXPR23 token_base &operator=(token_base &&rhs) noexcept = default;

  EF_CONSTEXPR23 token_base(token_base const &rhs) noexcept { assign(rhs); }
  EF_CONSTEXPR23 token_base &operator=(token_base const &rhs) noexcept { return assign(rhs); }

  EF_CONSTEXPR23 token_base clone() const {
    auto ret = token_base(make_data(*data_));
    if constexpr (is_shared_storage) {
      ret.data_->ref_count = 1;
    }
    return ret;
  }

  EF_CONSTEXPR23 static [[nodiscard]] token_base uninitialized() noexcept { return token_base{}; }
  EF_CONSTEXPR23 static [[nodiscard]] token_base null() { return token_base(make_data()); }

  EF_CONSTEXPR23 static [[nodiscard]] token_base make(std::string_view lit) {
    return make(0, lit, 0, 0, std::string_view{});
  }
  EF_CONSTEXPR23 static [[nodiscard]] token_base make(size_t id, std::string_view lit, size_t line,
                                                      size_t col, std::string_view fname,
                                                      bool end_of_file = false) {
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

  EF_CONSTEXPR23 [[nodiscard]] bool is_uninitialized() const noexcept {
    return static_cast<bool>(data_);
  }

  EF_CONSTEXPR23 [[nodiscard]] bool is_valid() const noexcept {
    return !is_uninitialized() && !lit().empty();
  }

  EF_CONSTEXPR23 [[nodiscard]] size_t id() const noexcept { return data_->id; }
  EF_CONSTEXPR23 [[nodiscard]] std::string_view lit() const noexcept { return data_->lit; }
  EF_CONSTEXPR23 [[nodiscard]] code_span span() const noexcept { return data_->span; }
  EF_CONSTEXPR23 [[nodiscard]] std::string_view file_name() const noexcept {
    return data_->file_name;
  }
  EF_CONSTEXPR23 [[nodiscard]] bool end_of_file() const { return data_->end_of_file; }

private:
  EF_CONSTEXPR23 token_base() = default;
  EF_CONSTEXPR23 explicit token_base(data_ptr data) noexcept : data_{std::move(data)} {}
};

using unique_token = token_base<token_storage_policy::unique>;
using shared_token = token_base<token_storage_policy::shared>;
using token = shared_token;

} // namespace sasl::common
