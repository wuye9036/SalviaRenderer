#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/semantic_analyser_impl.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::node;
using ::sasl::common::compiler_info_manager;

void semantic_analysis( boost::shared_ptr<node> root, boost::shared_ptr<compiler_info_manager> cinfo_mgr ){
	semantic_analyser_impl saimpl( cinfo_mgr );
	root->accept(&saimpl, NULL);
}

END_NS_SASL_SEMANTIC();