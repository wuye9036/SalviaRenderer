#include <sasl/common/token.h>

using std::make_shared;
using std::shared_ptr;
using std::string_view;

namespace sasl::common {

token_t::token_t() : s("UNINITIALIZED_VALUE"), end_of_file(false) {}

token_t::token_t(const token_t &rhs)
    : file_name(rhs.file_name), span(rhs.span), s(rhs.s), id(rhs.id), end_of_file(rhs.end_of_file) {}

std::shared_ptr<token_t> token_t::make(size_t id, std::string_view s, size_t line, size_t col, std::string_view fname) {
  std::shared_ptr<token_t> ret = std::make_shared<token_t>();
  ret->id = id;
  ret->s = s;
  ret->file_name = fname;
  ret->span = inline_code_span(line, col, col);
  ret->end_of_file = false;

  size_t cur_line = line;
  size_t cur_col = col;
  for (auto ch : s) {
    if (ch == '\n') {
      ++cur_line;
      cur_col = 1;
    } else {
      ++cur_col;
    }
  }

  ret->span.end = {cur_line, cur_col};

  return ret;
}

shared_ptr<token_t> token_t::make_copy() const { return shared_ptr<token_t>(new token_t(*this)); }

token_t &token_t::operator=(const token_t &rhs) {
  file_name = rhs.file_name;
  span = rhs.span;
  s = rhs.s;
  end_of_file = rhs.end_of_file;
  id = rhs.id;
  return *this;
}

shared_ptr<token_t> token_t::null() { return shared_ptr<token_t>(); }

shared_ptr<token_t> token_t::from_string(string_view str) {
  return shared_ptr<token_t>(new token_t(str.begin(), str.end()));
}

} // namespace sasl::common