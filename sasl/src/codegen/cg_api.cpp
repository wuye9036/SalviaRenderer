#include <sasl/include/codegen/cg_api.h>

#include <sasl/include/codegen/cg_general.h>
#include <sasl/include/codegen/cg_vs.h>
#include <sasl/include/codegen/cg_ps.h>

#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/node.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODEGEN();

using sasl::semantic::module_semantic;
using sasl::semantic::symbol;
using sasl::semantic::abi_info;
using sasl::syntax_tree::node;
using boost::shared_ptr;

boost::shared_ptr<cg_module> generate_llvm_code(
	boost::shared_ptr<sasl::semantic::module_semantic> const& mod,
	sasl::semantic::abi_info const* abii )
{
	shared_ptr<cg_module> ret;
	
	symbol* root = mod->root_symbol();
	if(!root) { return ret; }

	node* assoc_node = root->associated_node();
	if(!assoc_node) { return ret; }
	if(assoc_node->node_class() != node_ids::program) { return ret; }
	
	if( !abii || abii->lang == salviar::lang_general ){
		cg_general cg;
		if( cg.generate(mod, abii) ){
			return cg.generated_module();
		}
	}
		
	if ( abii->lang == salviar::lang_vertex_shader ){
		cg_vs cg;
		if( cg.generate(mod, abii) ){
			return cg.generated_module();
		}
	}

	if( abii->lang == salviar::lang_pixel_shader ){
		cg_ps cg;
		if( cg.generate(mod, abii) ){
			return cg.generated_module();
		}
	}

	return ret;
}

END_NS_SASL_CODEGEN();
