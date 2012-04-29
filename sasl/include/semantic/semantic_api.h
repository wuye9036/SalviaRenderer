#ifndef SASL_SEMANTIC_SEMANTIC_API_H
#define SASL_SEMANTIC_SEMANTIC_API_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
	namespace common{
		class diag_chat;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class module_si;

boost::shared_ptr<sasl::semantic::module_si> analysis_semantic( boost::shared_ptr<sasl::syntax_tree::node> const& root, sasl::common::diag_chat*, uint32_t lang );
boost::shared_ptr<sasl::semantic::module_si> analysis_semantic( sasl::syntax_tree::node* root, sasl::common::diag_chat*, uint32_t lang );

END_NS_SASL_SEMANTIC();

#endif
