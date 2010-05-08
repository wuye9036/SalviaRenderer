#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/semantic_analyser_impl.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_SEMANTIC();

void semantic_analysis( boost::shared_ptr<::sasl::syntax_tree::node> root ){
	semantic_analyser_impl saimpl;
	root->accept(&saimpl);
}

END_NS_SASL_SEMANTIC();