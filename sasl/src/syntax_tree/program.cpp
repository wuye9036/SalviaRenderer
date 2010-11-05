#include <sasl/include/syntax_tree/program.h>

#include <sasl/enums/syntax_node_types.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

program::program( const std::string& name)
	: node( syntax_node_types::program, boost::shared_ptr<token_attr>() ), name(name)
{
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( program );

program& program::d( boost::shared_ptr<declaration> decl ){
	decls.push_back( decl );
	return *this;
}
END_NS_SASL_SYNTAX_TREE();