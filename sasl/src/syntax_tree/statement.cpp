#include <sasl/common/token.h>
#include <sasl/syntax_tree/statement.h>
#include <sasl/syntax_tree/visitor.h>

#include <cassert>

using std::shared_ptr;

namespace sasl::syntax_tree {
std::shared_ptr<struct label> labeled_statement::pop_label() {
  assert(!labels.empty());
  std::shared_ptr<struct label> ret = labels.back();
  labels.pop_back();
  return ret;
}

void labeled_statement::push_label(std::shared_ptr<label> lbl) { this->labels.push_back(lbl); }

} // namespace sasl::syntax_tree