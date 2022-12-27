#include <sasl/common/diag_chat.h>
#include <sasl/parser/diags.h>
#include <sasl/parser/error_handlers.h>
#include <sasl/parser/generator.h>

using sasl::common::diag_chat;
using namespace sasl::parser::diags;

namespace sasl::parser {

error_handler get_expected_failed_handler(std::string const &expected_str) {
  return [=](diag_chat *diags, token_iterator const &org_iter, token_iterator &iter) {
    return expected_failed_handler(diags, org_iter, iter, expected_str);
  };
}

parse_results expected_failed_handler(diag_chat *diags, token_iterator const &org_iter,
                                      token_iterator &iter, std::string const &expected_str) {
  token const &tok = *iter;

  diags->clear();
  if (tok.end_of_file()) {
    diags->report(end_of_file, tok.file_name(), tok.span(), expected_str);
  } else {
    diags->report(unmatched_expected_token, tok.file_name(), tok.span(), expected_str, tok.lit());
  }

  iter = org_iter;

  return parse_results::recovered_expected_failed;
}

} // namespace sasl::parser