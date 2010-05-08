#ifndef SASL_SEMANTIC_SEMANTIC_ANALYSER_H
#define SASL_SEMANTIC_SEMANTIC_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

void semantic_analysis( boost::shared_ptr<::sasl::syntax_tree::node> root );

END_NS_SASL_SEMANTIC();

#endif