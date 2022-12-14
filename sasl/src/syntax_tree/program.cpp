#include <sasl/syntax_tree/program.h>

#include <sasl/enums/node_ids.h>
#include <sasl/syntax_tree/visitor.h>

namespace sasl::syntax_tree {

program::program( const std::string& name)
	: node( node_ids::program, std::shared_ptr<token_t>(), std::shared_ptr<token_t>() ), name(name)
{
}

program::program( std::shared_ptr<token_t> const& tok_beg, std::shared_ptr<token_t> const& tok_end )
	: node( node_ids::program, tok_beg, tok_end )
{
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( program );

}