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
	boost::shared_ptr< ::sasl::syntax_tree::tynode > lhs,
	boost::shared_ptr< ::sasl::syntax_tree::tynode > rhs
);

bool type_equal(
	boost::shared_ptr< ::sasl::syntax_tree::builtin_type > lhs,
	boost::shared_ptr< ::sasl::syntax_tree::builtin_type > rhs
);

// boost::shared_ptr<::sasl::syntax_tree::tynode> actual_type( boost::shared_ptr<::sasl::syntax_tree::tynode> );
END_NS_SASL_SEMANTIC();

#endif