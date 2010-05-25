#ifndef SASL_SEMANTIC_TYPE_CHECKER_H
#define SASL_SEMANTIC_TYPE_CHECKER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace syntax_tree{
		struct type_specifier;
		struct function_type;
	}
}

BEGIN_NS_SASL_SEMANTIC();

std::string mangle_function_name( boost::shared_ptr<::sasl::syntax_tree::function_type> v );

bool is_equal(
	boost::shared_ptr<::sasl::syntax_tree::type_specifier> lhs,
	boost::shared_ptr<::sasl::syntax_tree::type_specifier> rhs
	);

bool is_equal(
	boost::shared_ptr<::sasl::syntax_tree::function_type> lhs,
	boost::shared_ptr<::sasl::syntax_tree::function_type> rhs
	);

boost::shared_ptr<::sasl::syntax_tree::type_specifier> actual_type( boost::shared_ptr<::sasl::syntax_tree::type_specifier> );
END_NS_SASL_SEMANTIC();

#endif