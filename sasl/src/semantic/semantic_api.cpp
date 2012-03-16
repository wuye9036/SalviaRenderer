#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/node.h>

using sasl::syntax_tree::node;
using boost::shared_ptr;

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<module_si> analysis_semantic( shared_ptr<node> const& root ){
	semantic_analyser saimpl;
	root->accept(&saimpl, NULL);
	return saimpl.module_semantic_info();
}

shared_ptr<module_si> analysis_semantic( node* root )
{
	return analysis_semantic( root->as_handle() );
}

END_NS_SASL_SEMANTIC();

