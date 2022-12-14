#include <sasl/semantic/semantic_api.h>
#include <sasl/semantic/semantic_analyser.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/common/diag_chat.h>

using sasl::common::diag_chat;
using sasl::syntax_tree::node;
using std::shared_ptr;

namespace sasl::semantic {

shared_ptr<module_semantic> analysis_semantic( shared_ptr<node> const& root, diag_chat* diags, uint32_t lang ){
	semantic_analyser saimpl;
	saimpl.language(lang);
	root->accept(&saimpl, nullptr);
	diag_chat::merge( diags, saimpl.get_diags(), true );
	return saimpl.get_module_semantic();
}

shared_ptr<module_semantic> analysis_semantic( node* root, diag_chat* diags, uint32_t lang )
{
	return analysis_semantic( root->as_handle(), diags, lang );
}

}

