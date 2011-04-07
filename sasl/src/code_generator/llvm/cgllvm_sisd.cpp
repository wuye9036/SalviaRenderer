#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>

#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>

#include <sasl/enums/enums_helper.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/BasicBlock.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace llvm;
using namespace sasl::syntax_tree;

using boost::addressof;
using boost::any_cast;

using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_sisd

BEGIN_NS_SASL_CODE_GENERATOR();

bool cgllvm_sisd::generate( sasl::semantic::module_si* mod, sasl::semantic::abi_info const* abii ){
	this->msi = mod;
	this->abii = abii;

	if ( msi ){
		assert( msi->root() );
		assert( msi->root()->node() );

		msi->root()->node()->accept( this, NULL );
		return true;
	}

	return false;
}

SASL_VISIT_DEF( program ){
	// Create module.
	if( !create_mod( v ) ){
		return;
	}

	// Initialization.
	before_decls_visit( v, data );

	// visit declarations
	any child_ctxt = cgllvm_sctxt();
	for( vector< shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child( child_ctxt, (*it) );
	}
}

// Generate normal function code.
SASL_VISIT_DEF( function_type ){
	sc_ptr(data)->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	// Generate return types.
	visit_child( child_ctxt, child_ctxt_init, v.retval_type );
	Type const* ret_type = sc_inner_ptr(data)->val_type;

	EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){
		return;
	}

	// Generate paramenter types.
	vector<Type const*> param_types;
	for( vector< shared_ptr<parameter> >::iterator it = v.params.begin(); it != v.params.end(); ++it ){
		visit_child( child_ctxt, child_ctxt_init, *it );
		if ( sc_inner_ptr(&child_ctxt)->val_type ){
			param_types.push_back( sc_inner_ptr(&child_ctxt)->val_type );
		} else {
			EFLIB_ASSERT_AND_IF( ret_type, "Error occurs while parameter parsing." ){
				return;
			}
		}
	}

	// Create function.
	FunctionType* ftype = FunctionType::get( ret_type, param_types, false );
	sc_inner_ptr(data)->val_type = ftype;

	Function* fn = 
		Function::Create( ftype, Function::ExternalLinkage, v.name->str, mod_ptr()->module() );
	sc_inner_ptr(data)->self_fn = fn;
	sc_inner_ptr(&child_ctxt_init)->parent_fn = fn;

	// Register parameter names.
	Function::arg_iterator arg_it = fn->arg_begin();
	for( size_t arg_idx = 0; arg_idx < fn->arg_size(); ++arg_idx, ++arg_it){
		shared_ptr<parameter> par = v.params[arg_idx];
		arg_it->setName( par->symbol()->unmangled_name() );
		sctxt_handle par_ctxt = node_ctxt( par );
		par_ctxt->data().val = arg_it;
	}

	// Create function body.
	if ( v.body ){
		visit_child( child_ctxt, child_ctxt_init, v.body );
		clear_empty_blocks( fn );
	}

	*node_ctxt(v, true) = *( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program ){
	mod_ptr()->create_module( v.name );

	ctxt_getter = boost::bind( &cgllvm_sisd::node_ctxt<node>, this, _1, false );
	typeconv = create_type_converter( mod_ptr()->builder(), ctxt_getter );
	register_builtin_typeconv( typeconv, msi->type_manager() );
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

cgllvm_sctxt_data* sc_inner_ptr( boost::any* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

cgllvm_sctxt_data const* sc_inner_ptr( boost::any const* any_val ){
	return addressof( sc_ptr(any_val)->data() );
}

Constant* cgllvm_sisd::zero_value( boost::shared_ptr<type_specifier> typespec )
{
	if( typespec->node_class() == syntax_node_types::builtin_type ){
		builtin_type_code btc = typespec->value_typecode;
		Type const* valtype = node_ctxt(typespec)->data().val_type;
		if( sasl_ehelper::is_integer( btc ) ){
			return ConstantInt::get( valtype, 0, sasl_ehelper::is_signed(btc) );
		}
		if( sasl_ehelper::is_real( btc ) ){
			return ConstantFP::get( valtype, 0.0 );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

llvm::Constant* cgllvm_sisd::zero_value( llvm::Type const* t ){
	return Constant::getNullValue(t);
}

cgllvm_sctxt* cgllvm_sisd::node_ctxt( sasl::syntax_tree::node& v, bool create_if_need /*= false */ )
{
	return cgllvm_impl::node_ctxt<cgllvm_sctxt>(v, create_if_need);
}

void cgllvm_sisd::restart_block( boost::any* data ){
	BasicBlock* restart = BasicBlock::Create( llcontext(), "", sc_inner_ptr(data)->parent_fn );
	builder()->SetInsertPoint(restart);
}

cgllvm_modimpl* cgllvm_sisd::mod_ptr(){
	assert( dynamic_cast<cgllvm_modimpl*>( mod.get() ) );
	return static_cast<cgllvm_modimpl*>( mod.get() );
}

llvm::Value* cgllvm_sisd::load( boost::any* data ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

llvm::Value* cgllvm_sisd::load( cgllvm_sctxt* data ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

void cgllvm_sisd::store( llvm::Value*, boost::any* data ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgllvm_sisd::store( llvm::Value*, cgllvm_sctxt* data ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}


void cgllvm_sisd::create_alloca( cgllvm_sctxt* ctxt, std::string const& name ){
	assert( ctxt );
	assert( ctxt->data().val_type );

	if( !ctxt->data().global ){
		Function* parent_fn = ctxt->data().parent_fn;
		assert( parent_fn );
		IRBuilder<> vardecl_builder( mod_ptr()->context() ) ;
		vardecl_builder.SetInsertPoint( &parent_fn->getEntryBlock(), parent_fn->getEntryBlock().begin() );
		ctxt->data().local
			= builder()->CreateAlloca( sc_ptr(ctxt)->data().val_type, 0, name.c_str() );
	} else {
		ctxt->data().global
			 = cast<GlobalVariable>( mod_ptr()->module()->getOrInsertGlobal( name, ctxt->data().val_type ) );
	}
}

void cgllvm_sisd::clear_empty_blocks( llvm::Function* fn )
{
	// Inner empty block, insert an br instruction for jumping to next block.
	for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin();
		it != fn->getBasicBlockList().end(); ++it
		)
	{
		if( it->empty() ){
			Function::BasicBlockListType::iterator next_it = it;
			++next_it;

			if( next_it != fn->getBasicBlockList().end() ){
				mod_ptr()->builder()->CreateBr( &(*next_it) );
			} else {
				Value* val = zero_value( fn->getReturnType() );
				mod_ptr()->builder()->CreateRet(val);
			}
		}
	}
}

END_NS_SASL_CODE_GENERATOR();