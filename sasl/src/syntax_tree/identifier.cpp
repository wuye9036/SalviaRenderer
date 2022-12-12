#include <sasl/syntax_tree/identifier.h>
#include <sasl/common/token.h>
#include <sasl/syntax_tree/visitor.h>
namespace sasl::syntax_tree() {

using namespace boost;

identifier::identifier( std::shared_ptr<token_t> const& tok )
	: node( node_ids::identifier, tok, tok ), name(tok->str){}

}