#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/semantic_analyser_impl.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::node;
using ::sasl::common::compiler_info_manager;

using boost::shared_ptr;

shared_ptr<module_si> semantic_analysis( shared_ptr<node> root, softart::languages lang ){
	semantic_analyser_impl saimpl;
	saimpl.language( lang );
	root->accept(&saimpl, NULL);
	return saimpl.global_semantic_info();
}

END_NS_SASL_SEMANTIC();