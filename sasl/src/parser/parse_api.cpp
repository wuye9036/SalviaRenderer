#include <sasl/parser/parse_api.h>

#include <sasl/common/diag_chat.h>
#include <sasl/common/lex_context.h>
#include <sasl/parser/diags.h>
#include <sasl/parser/grammars.h>
#include <sasl/parser/lexer.h>

#include <eflib/diagnostics/assert.h>

#include <iostream>

namespace sasl {
namespace common {
class diag_chat;
}
} // namespace sasl

using namespace sasl::parser::diags;

using sasl::common::code_source;
using sasl::common::diag_chat;
using sasl::common::inline_code_span;
using sasl::common::lex_context;

using std::cout;
using std::endl;
using std::shared_ptr;

namespace sasl::parser {
bool parse(shared_ptr<attribute> &pt_root, const std::string &code, shared_ptr<lex_context> ctxt,
           lexer &l, grammars &g, diag_chat *diags) {
  token_seq toks;

  bool tok_result = l.tokenize_with_end(code, ctxt, toks);
  if (!tok_result) {
    diags->report(unrecognized_token, ctxt->file_name(),
                  inline_code_span(ctxt->line(), ctxt->column(), 1),
                  fmt::arg("token", "<unknown>"));
    return false;
  }

  token_iterator it = toks.begin();
  return g.prog.parse(it, toks.end() - 1, pt_root, diags).is_succeed();
}

bool parse(shared_ptr<attribute> &pt_root, code_source *src, shared_ptr<lex_context> ctxt, lexer &l,
           grammars &g, diag_chat *diags) {
  token_seq toks;

  l.begin_incremental();
  try {
    while (!src->eof() && !src->failed()) {
      std::string next_token{src->next()};
      bool tok_result = l.incremental_tokenize(next_token, ctxt, toks);
      if (!tok_result) {
        diags->report(unrecognized_token, ctxt->file_name(),
                      inline_code_span(ctxt->line(), ctxt->column(), 1),
                      fmt::arg("token", next_token));
        return false;
      }
    }
  } catch (...) {
    l.end_incremental(ctxt, toks);
    return false;
  }

  l.end_incremental(ctxt, toks);
  token_iterator it = toks.begin();
  return !src->failed() && g.prog.parse(it, toks.end() - 1, pt_root, diags).is_succeed();
}
} // namespace sasl::parser
