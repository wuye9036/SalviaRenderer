#pragma once

#include <memory>
#include <string_view>

namespace sasl::common {

struct code_span {
  code_span();
  code_span(size_t line_beg, size_t col_beg, size_t line_end, size_t col_end);
  code_span(size_t line_beg, size_t col_beg, size_t length);
  void set(size_t line_beg, size_t col_beg, size_t line_end, size_t col_end);
  void set(size_t line_beg, size_t col_beg, size_t length);
  size_t line_beg, col_beg;
  size_t line_end, col_end;
  static code_span merge(code_span const &s0, code_span const &s1);
};

struct token_t {
  token_t();

  token_t(const token_t &rhs);

  template <typename IteratorT>
  token_t(IteratorT const &first, IteratorT const &last)
      : s(first, last), id(0), end_of_file(false) {}

  token_t &operator=(const token_t &rhs);

  std::shared_ptr<token_t> make_copy() const;

  static std::shared_ptr<token_t> null();
  static std::shared_ptr<token_t> from_string(std::string_view s);
  static std::shared_ptr<token_t> make(size_t id, std::string_view s,
                                       size_t line, size_t col,
                                       std::string_view fname);

  size_t id;
  std::string_view s;
  code_span span;
  std::string_view file_name;
  bool end_of_file;
};

} // namespace sasl::common
