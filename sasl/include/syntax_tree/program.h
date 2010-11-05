#ifndef SASL_SYNTAX_TREE_PROGRAM_H
#define SASL_SYNTAX_TREE_PROGRAM_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <sasl/include/syntax_tree/node.h>
#include <eflib/include/metaprog/enable_if.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <vector>

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_visitor;
struct declaration;

struct program: public node{

	SASL_SYNTAX_NODE_CREATORS();

	// help for creating program syntax tree

	program& d( boost::shared_ptr<declaration> );
	template < typename T >
	program& d( boost::shared_ptr<T> decl, EFLIB_ENABLE_IF_PRED2( is_base_of, declaration, T, 0 ) ){
		return d( boost::shared_polymorphic_cast<declaration>(decl) );
	}

	SASL_SYNTAX_NODE_ACCEPT_METHOD_DECL();
	std::string name;
	std::vector< boost::shared_ptr<declaration> > decls;

protected:
	program(const std::string& name);
	program& operator = (const program&);
	program( const program& );
};

END_NS_SASL_SYNTAX_TREE();

#endif