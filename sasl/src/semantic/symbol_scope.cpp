#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/symbol.h>

BEGIN_NS_SASL_SEMANTIC();

symbol_scope::symbol_scope( const std::string& child_name, boost::shared_ptr<symbol>& sym )
	: cursym( sym )
{
	sym = sym->find_this( child_name );
}

symbol_scope::symbol_scope(
		const std::string& child_name,
		boost::shared_ptr<node> child_node,
		boost::shared_ptr<symbol>& sym
		): cursym( sym )
{
	sym = sym->add_child( child_name, child_node );
}

symbol_scope::~symbol_scope(){
	cursym = cursym->parent();
}

END_NS_SASL_SEMANTIC();