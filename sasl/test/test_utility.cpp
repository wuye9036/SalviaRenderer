#include "test_utility.h"
#include <sasl/include/common/token_attr.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/semantic/symbol.h>

using ::sasl::syntax_tree::create_node;

template <typename SymbolInfoT>
void extract_symbol_info( boost::shared_ptr<SymbolInfoT>& syminfo, boost::shared_ptr<symbol> sym ){
	syminfo = sym->symbol_info<SymbolInfoT>();
}