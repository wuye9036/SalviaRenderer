#ifndef SASL_SEMANTIC_TYPE_CHECKER_H
#define SASL_SEMANTIC_TYPE_CHECKER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace syntax_tree{
		struct type_specifier;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::type_specifier;

bool is_equal( boost::shared_ptr<type_specifier> type0, boost::shared_ptr<type_specifier> type1 );

END_NS_SASL_SEMANTIC();

#endif