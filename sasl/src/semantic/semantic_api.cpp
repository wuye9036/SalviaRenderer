#include <sasl/include/semantic/semantic_api.h>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/common/diag_chat.h>

using sasl::common::diag_chat;
using sasl::syntax_tree::node;
using boost::shared_ptr;

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<module_si> analysis_semantic( shared_ptr<node> const& root, diag_chat* diags ){
	semantic_analyser saimpl;
	root->accept(&saimpl, NULL);
	diag_chat::merge( diags, saimpl.get_diags(), true );
	return saimpl.module_semantic_info();
}

shared_ptr<module_si> analysis_semantic( node* root, diag_chat* diags )
{
	return analysis_semantic( root->as_handle(), diags );
}

END_NS_SASL_SEMANTIC();

