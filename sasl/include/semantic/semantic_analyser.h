#ifndef SASL_SEMANTIC_SEMANTIC_ANALYSER_H
#define SASL_SEMANTIC_SEMANTIC_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
	namespace common{
		class compiler_info_manager;
	}
}

BEGIN_NS_SASL_SEMANTIC();

void semantic_analysis(
	boost::shared_ptr<::sasl::syntax_tree::node> root,
	boost::shared_ptr<::sasl::common::compiler_info_manager> cinfo_mgr
	);

END_NS_SASL_SEMANTIC();

#endif