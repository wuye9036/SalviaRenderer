#ifndef SASL_SEMANTIC_SYMBOL_SCOPE_H
#define SASL_SEMANTIC_SYMBOL_SCOPE_H

#include <sasl/include/semantic/semantic_forward.h>
#include <string>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;
using ::sasl::syntax_tree::node;

class symbol_scope{
public:
	symbol_scope(
		const std::string& child_name,
		boost::shared_ptr<symbol>& sym
		);

	symbol_scope(
		const std::string& child_name,
		boost::shared_ptr<node> child_node,
		boost::shared_ptr<symbol>& sym
		);
	
	symbol_scope(
		const ::std::string& mangled_child_name,
		const ::std::string& unmangled_child_name,
		boost::shared_ptr<node> child_node,
		boost::shared_ptr<symbol>& sym
		);

	~symbol_scope();

private:
	symbol_scope& operator = (const symbol_scope& );
	boost::shared_ptr<symbol>& cursym;
};

END_NS_SASL_SEMANTIC();
#endif