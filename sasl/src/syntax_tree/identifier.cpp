#include <sasl/common/token.h>
#include <sasl/syntax_tree/identifier.h>
#include <sasl/syntax_tree/visitor.h>
namespace sasl::syntax_tree {

identifier::identifier(std::shared_ptr<token_t> const &tok) : node(node_ids::identifier, tok, tok), name(tok->s) {}

} // namespace sasl::syntax_tree