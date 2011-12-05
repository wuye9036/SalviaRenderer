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
#include <llvm/Target/TargetData.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using namespace llvm;
using namespace sasl::utility;

using boost::bind;
using boost::any_cast;
using boost::addressof;

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

cgllvm_impl::~cgllvm_impl()
{
	if( target_data ){ delete target_data; }
}

SASL_VISIT_DEF( builtin_type ){

	shared_ptr<type_info_si> tisi = extract_semantic_info<type_info_si>( v );

	cgllvm_sctxt* pctxt = node_ctxt( tisi->type_info(), true );

	if( !pctxt->get_typtr() ){
		shared_ptr<value_tyinfo> bt_tyinfo = service()->create_tyinfo( v.as_handle<tynode>() );
		assert( bt_tyinfo );
		pctxt->data().tyinfo = bt_tyinfo;

		std::string tips = v.tycode.name() + std::string(" was not supported yet.");
		EFLIB_ASSERT_AND_IF( pctxt->data().tyinfo, tips.c_str() ){
			return;
		}
	}

	sc_ptr( data )->data( pctxt );
	return;
}
SASL_VISIT_DEF( struct_type ){
	// Create context.
	// Declarator visiting need parent information.
	cgllvm_sctxt* ctxt = node_ctxt(v, true);

	// A struct is visited at definition type.
	// If the visited again, it must be as an alias_type.
	// So return environment directly.
	if( ctxt->data().tyinfo ){
		sc_ptr(data)->data(ctxt);
		return;
	}

	std::string name = v.symbol()->mangled_name();

	// Init data.
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	sc_env_ptr(&child_ctxt_init)->parent_struct = ctxt;

	any child_ctxt;
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child( child_ctxt, child_ctxt_init, decl );
	}
	sc_data_ptr(data)->tyinfo = service()->create_tyinfo( v.si_ptr<type_info_si>()->type_info() );

	ctxt->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_declaration ){
	// Visit type info
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.type_info );

	sc_env_ptr(&child_ctxt_init)->tyinfo = sc_data_ptr(&child_ctxt)->tyinfo;

	BOOST_FOREACH( shared_ptr<declarator> const& dclr, v.declarators ){
		visit_child( child_ctxt, child_ctxt_init, dclr );
	}

	sc_data_ptr(data)->declarator_count = static_cast<int>( v.declarators.size() );

	sc_data_ptr(data)->tyinfo = sc_data_ptr(&child_ctxt)->tyinfo;
	node_ctxt(v, true)->copy( sc_ptr(data) );
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

	typedef cgllvm_sctxt* (fn_proto_t)( boost::shared_ptr<sasl::syntax_tree::node> const& );
	typedef cgllvm_sctxt* (cgllvm_impl::*mem_fn_proto_t) ( boost::shared_ptr<sasl::syntax_tree::node> const&, bool );
	boost::function<fn_proto_t> ctxt_getter
		= boost::bind( static_cast<mem_fn_proto_t>(&cgllvm_impl::node_ctxt), this, _1, false );
	caster = create_caster( ctxt_getter, service() );
	add_builtin_casts( caster, msi->pety() );
	
	process_intrinsics( v, data );

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

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	target_data = new TargetData( module() );
}

SASL_SPECIFIC_VISIT_DEF( process_intrinsics, program )
{
	vector< shared_ptr<symbol> > const& intrinsics = msi->intrinsics();

	BOOST_FOREACH( shared_ptr<symbol> const& intr, intrinsics ){
		shared_ptr<function_type> intr_fn = intr->node()->as_handle<function_type>();
		
		// If intrinsic is not invoked, we don't generate code for it.
		if( ! intr_fn->si_ptr<storage_si>()->is_invoked() ){
			continue;
		}

		any child_ctxt = cgllvm_sctxt();

		visit_child( child_ctxt, intr_fn );

		cgllvm_sctxt* intrinsic_ctxt = node_ctxt( intr_fn, false );
		assert( intrinsic_ctxt );

		service()->push_fn( intrinsic_ctxt->data().self_fn );
		scope_guard<void> pop_fn_on_exit( bind( &cg_service::pop_fn, service() ) );

		insert_point_t ip_body = service()->new_block( ".body", true );

		// Parse Parameter Informations
		vector< shared_ptr<tynode> > par_tys;
		vector<builtin_types> par_tycodes;
		vector<cgllvm_sctxt*> par_ctxts;

		BOOST_FOREACH( shared_ptr<parameter> const& par, intr_fn->params )
		{
			par_tys.push_back( par->si_ptr<type_info_si>()->type_info() );
			assert( par_tys.back() );
			par_tycodes.push_back( par_tys.back()->tycode );
			par_ctxts.push_back( node_ctxt(par, false) );
			assert( par_ctxts.back() );
		}

		shared_ptr<value_tyinfo> result_ty = service()->fn().get_return_ty();
		
		service()->fn().inline_hint();

		// Process Intrinsic
		if( intr->unmangled_name() == "mul" ){
			
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_mul( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, abi_llvm );

		} else if( intr->unmangled_name() == "dot" ) {
			
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_dot( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, abi_llvm );

		} else if( intr->unmangled_name() == "sqrt" ){
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_sqrt( service()->fn().arg(0) );
			service()->emit_return( ret_val, abi_llvm );
		} else if( intr->unmangled_name() == "cross" ){
			assert( par_tys.size() == 2 );
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );
			value_t ret_val = service()->emit_cross( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, abi_llvm );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}
}

cgllvm_sctxt const * sc_ptr( const boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

cgllvm_sctxt* sc_ptr( boost::any& any_val ){
	return any_cast<cgllvm_sctxt>(&any_val);
}

cgllvm_sctxt const * sc_ptr( const boost::any* any_val )
{
	return any_cast<cgllvm_sctxt>(any_val);
}

cgllvm_sctxt* sc_ptr( boost::any* any_val )
{
	return any_cast<cgllvm_sctxt>(any_val);
}

cgllvm_sctxt_data* sc_data_ptr( boost::any* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

cgllvm_sctxt_data const* sc_data_ptr( boost::any const* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

cgllvm_sctxt_env* sc_env_ptr( boost::any* any_val ){
	return addressof( sc_ptr(any_val)->env() );
}

cgllvm_sctxt_env const* sc_env_ptr( boost::any const* any_val ){
	return addressof( sc_ptr(any_val)->env() );
}

END_NS_SASL_CODE_GENERATOR();