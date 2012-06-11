#ifndef SASL_SEMANTIC_SEMANTIC_API_H
#define SASL_SEMANTIC_SEMANTIC_API_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

namespace sasl{
	namespace syntax_tree{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
	}
	namespace common{
		class diag_chat;
	}
}

BEGIN_NS_SASL_SEMANTIC();

EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);

module_semantic_ptr analysis_semantic( sasl::syntax_tree::node_ptr const& root, sasl::common::diag_chat*, uint32_t lang );
module_semantic_ptr analysis_semantic( sasl::syntax_tree::node* root, sasl::common::diag_chat*, uint32_t lang );

END_NS_SASL_SEMANTIC();

#endif
