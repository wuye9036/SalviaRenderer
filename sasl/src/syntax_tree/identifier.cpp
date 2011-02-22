#include <sasl/include/syntax_tree/identifier.h>
#include <sasl/include/common/token.h>
#include <sasl/include/syntax_tree/visitor.h>
BEGIN_NS_SASL_SYNTAX_TREE();

using namespace boost;

identifier::identifier( boost::shared_ptr<token_t> tok )
	: node( syntax_node_types::identifier, tok ), name(tok->str){}

END_NS_SASL_SYNTAX_TREE();