#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/symbol.h>

BEGIN_NS_SASL_SEMANTIC();

symbol_scope::symbol_scope(
		const std::string& unmangled,
		boost::shared_ptr<node> child_node,
		boost::shared_ptr<symbol>& sym
		): cursym( sym )
{
	if( !unmangled.empty() ){
		sym = sym->add_child( unmangled, child_node );
	} else {
		sym = sym->add_anonymous_child( child_node );
	}
}

symbol_scope::~symbol_scope(){
	cursym = cursym->parent();
}

END_NS_SASL_SEMANTIC();