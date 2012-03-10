#ifndef SASL_SEMANTIC_SEMANTIC_ERROR_H
#define SASL_SEMANTIC_SEMANTIC_ERROR_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#define BEGIN_NS_SASL_SEMANTIC_ERRORS() namespace sasl{ namespace semantic{ namespace errors{
#define END_NS_SASL_SEMANTIC_ERRORS() } } }

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC_ERRORS();

END_NS_SASL_SEMANTIC_ERRORS();

#endif
