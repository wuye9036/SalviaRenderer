#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/node.h>

#include <softart/include/enums.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();

using sasl::semantic::module_si;
using sasl::semantic::symbol;
using sasl::semantic::abi_info;

using boost::shared_ptr;

boost::shared_ptr<llvm_code> generate_llvm_code( sasl::semantic::module_si* mod, sasl::semantic::abi_info const* abii )
{
	boost::shared_ptr<symbol> root = mod->root();
	if ( root && root->node() && root->node()->node_class() == syntax_node_types::program ){
		if( abii->lang == softart::lang_vertex_sl ){
			llvm_vscg cg;
			if( cg.generate(mod, abii) ){
				return cg.generated_module();
			}
		}
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
	return boost::shared_ptr<llvm_code>();
}

END_NS_SASL_CODE_GENERATOR();