#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#include <sasl/include/code_generator/llvm/cgllvm_general.h>
#include <sasl/include/code_generator/llvm/cgllvm_vs.h>
#include <sasl/include/code_generator/llvm/cgllvm_ps.h>

#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/node.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();

using sasl::semantic::module_semantic;
using sasl::semantic::symbol;
using sasl::semantic::abi_info;

using boost::shared_ptr;

boost::shared_ptr<llvm_module> generate_llvm_code( sasl::semantic::module_semantic* mod, sasl::semantic::abi_info const* abii )
{
	boost::shared_ptr<symbol> root = mod->root_symbol();
	if ( root && root->node() && root->node()->node_class() == node_ids::program ){
		
		if( !abii || abii->lang == salviar::lang_general ){
			cgllvm_general cg;
			if( cg.generate(mod, abii) ){
				return cg.cg_module();
			}
		}
		
		if ( abii->lang == salviar::lang_vertex_shader ){
			cgllvm_vs cg;
			if( cg.generate(mod, abii) ){
				return cg.cg_module();
			}
		}

		if( abii->lang == salviar::lang_pixel_shader ){
			cgllvm_ps cg;
			if( cg.generate(mod, abii) ){
				return cg.cg_module();
			}
		}
	}
	return boost::shared_ptr<llvm_module>();
}

END_NS_SASL_CODE_GENERATOR();