#include "test_utility.h"
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/semantic/symbol.h>

using ::sasl::syntax_tree::create_node;

boost::shared_ptr<token_attr> null_token(){
	return boost::shared_ptr<token_attr>();
}

boost::shared_ptr<token_attr> new_token( const std::string& lit ){
	return boost::shared_ptr<token_attr>( new token_attr(lit.begin(), lit.end()) );
}

boost::shared_ptr<buildin_type> new_buildin_type( boost::shared_ptr<token_attr> tok, buildin_type_code btc ){
	boost::shared_ptr<buildin_type> ret( create_node<buildin_type>( tok ) );
	ret->value_typecode = btc;
	return ret;
}

template <typename SymbolInfoT>
void extract_symbol_info( boost::shared_ptr<SymbolInfoT>& syminfo, boost::shared_ptr<symbol> sym ){
	syminfo = sym->symbol_info<SymbolInfoT>();
}