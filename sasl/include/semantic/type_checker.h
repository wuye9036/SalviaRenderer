#pragma once

#include <sasl/include/semantic/semantic_forward.h>

#include <string>
#include <memory>
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
