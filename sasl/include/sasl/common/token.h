#pragma once

#include <memory>
#include <string_view>

namespace sasl::common {

struct code_pos {
  size_t line, column;
};

constexpr std::weak_ordering operator<=>(code_pos lhs, code_pos rhs) {
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

struct token_t {
  token_t();

  token_t(const token_t &rhs);

  template <typename IteratorT>
  token_t(IteratorT const &first, IteratorT const &last) : s(first, last), id(0), end_of_file(false) {}

  token_t &operator=(const token_t &rhs);

  std::shared_ptr<token_t> make_copy() const;

  static std::shared_ptr<token_t> null();
  static std::shared_ptr<token_t> from_string(std::string_view s);
  static std::shared_ptr<token_t> make(size_t id, std::string_view s, size_t line, size_t col, std::string_view fname);

  size_t id;
  std::string_view s;
  code_span span;
  std::string_view file_name;
  bool end_of_file;
};

} // namespace sasl::common
