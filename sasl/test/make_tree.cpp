#include "make_tree.h"

#include <sasl/enums/buildin_type_code.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/declaration.h>

using ::sasl::common::token_attr;
using ::sasl::syntax_tree::buildin_type;
using ::sasl::syntax_tree::create_node;

boost::shared_ptr<token_attr> null_token(){
	return boost::shared_ptr<token_attr>();
}

boost::shared_ptr<token_attr> make_tree( const ::std::string& lit ){
	return boost::shared_ptr<token_attr>( new token_attr( lit.begin(), lit.end() ) );
}

boost::shared_ptr<buildin_type> make_tree( const buildin_type_code btc ){
	boost::shared_ptr<buildin_type> ret = create_node<buildin_type>( null_token() );
	ret->value_typecode = btc;
	return ret;
}