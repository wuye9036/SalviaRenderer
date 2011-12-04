#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/cgllvm_caster.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using namespace llvm;
using namespace sasl::utility;

using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::DefaultIRBuilder* cgllvm_impl::builder() const
{
	return mod->builder().get();
}

llvm::LLVMContext& cgllvm_impl::context() const
{
	return mod->context();
}

llvm::Module* cgllvm_impl::module() const
{
	return mod->module();
}

cgllvm_sctxt* cgllvm_impl::node_ctxt( node* n, bool create_if_need /*= false */ )
{
	shared_ptr<cgllvm_sctxt>& ret = ctxts[n];
	if( ret ){
		return ret.get();
	} else if( create_if_need ){
		ret = create_codegen_context<cgllvm_sctxt>( n->as_handle() );
		return ret.get();
	}

	return NULL;
}

shared_ptr<llvm_module> cgllvm_impl::cg_module() const
{
	return mod;
}

bool cgllvm_impl::generate( module_si* mod, abi_info const* abii )
{
	msi = mod;
	this->abii = abii;

	if( msi ){
		assert( msi->root() );
		assert( msi->root()->node() );
		msi->root()->node()->accept( this, NULL );
		return true;
	}

	return false;
}

SASL_VISIT_DEF( program )
{	
	// Create module.
	assert( !mod );
	mod = create_codegen_context<llvm_module_impl>( v.as_handle() );
	if( !mod ) return;

	// Initialization.
	mod->create_module( v.name );
	service()->initialize( mod.get(),
		boost::bind(static_cast<cgllvm_sctxt*(cgllvm_impl::*)(node*, bool)>(&cgllvm_impl::node_ctxt), this, _1, _2)
		);

	boost::function<cgllvm_sctxt*( boost::shared_ptr<sasl::syntax_tree::node> const& )> ctxt_getter
		= boost::bind( &cgllvm_impl::node_ctxt<node, cgllvm_sctxt>, this, _1, false );
	caster = create_caster( ctxt_getter, service() );
	add_builtin_casts( caster, msi->pety() );

	// Some other initializations.
	before_decls_visit( v, data );

	// visit declarations
	any child_ctxt = cgllvm_sctxt();
	for( vector< shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child( child_ctxt, (*it) );
	}
}

END_NS_SASL_CODE_GENERATOR();