#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::node;
using ::sasl::common::compiler_info_manager;

using boost::shared_ptr;

shared_ptr<module_si> analysis_semantic( shared_ptr<node> const& root ){
	semantic_analyser saimpl;
	root->accept(&saimpl, NULL);
	return saimpl.module_semantic_info();
}

END_NS_SASL_SEMANTIC();