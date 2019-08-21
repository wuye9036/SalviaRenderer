#ifndef SASL_SEMANTIC_TYPE_CHECKER_H
#define SASL_SEMANTIC_TYPE_CHECKER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace sasl{
	namespace syntax_tree{
		struct tynode;
		struct function_full_def;
		struct builtin_type;
	}
}

BEGIN_NS_SASL_SEMANTIC();

bool type_equal(
	std::shared_ptr< ::sasl::syntax_tree::tynode > lhs,
	std::shared_ptr< ::sasl::syntax_tree::tynode > rhs
);

bool type_equal(
	std::shared_ptr< ::sasl::syntax_tree::builtin_type > lhs,
	std::shared_ptr< ::sasl::syntax_tree::builtin_type > rhs
);

// std::shared_ptr<::sasl::syntax_tree::tynode> actual_type( std::shared_ptr<::sasl::syntax_tree::tynode> );
END_NS_SASL_SEMANTIC();

#endif