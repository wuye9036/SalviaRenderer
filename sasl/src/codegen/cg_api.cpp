#include <sasl/include/codegen/cg_api.h>

#include <sasl/include/codegen/cg_general.h>
#include <sasl/include/codegen/cg_vs.h>
#include <sasl/include/codegen/cg_ps.h>

#include <sasl/include/semantic/reflection_impl.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/node.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/shared_declaration.h>

BEGIN_NS_SASL_CODEGEN();

EFLIB_USING_SHARED_PTR(sasl::semantic, module_semantic);
EFLIB_USING_SHARED_PTR(sasl::semantic, reflection_impl);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);

using sasl::semantic::symbol;
using boost::shared_ptr;

module_vmcode_ptr generate_vmcode(
	module_semantic_ptr const&	sem,
	reflection_impl const*		reflection
	)
{
	module_vmcode_ptr ret;
	
	symbol* root = sem->root_symbol();
	if(!root) { return ret; }

	node* assoc_node = root->associated_node();
	if(!assoc_node) { return ret; }
	if(assoc_node->node_class() != node_ids::program) { return ret; }
	
	if(!reflection || reflection->get_language() == salviar::lang_general)
	{
		cg_general cg;
		if( cg.generate(sem, reflection) )
		{
			return cg.generated_module();
		}
	}
		
	if ( reflection->get_language() == salviar::lang_vertex_shader )
	{
		cg_vs cg;
		if( cg.generate(sem, reflection) )
		{
			return cg.generated_module();
		}
	}

	if( reflection->get_language() == salviar::lang_pixel_shader )
	{
		cg_ps cg;
		if( cg.generate(sem, reflection) )
		{
			return cg.generated_module();
		}
	}

	return ret;
}

END_NS_SASL_CODEGEN();
