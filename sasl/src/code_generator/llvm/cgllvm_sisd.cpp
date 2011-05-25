#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>

#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
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
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace llvm;
using namespace sasl::syntax_tree;

using sasl::semantic::extract_semantic_info;
using sasl::semantic::symbol;
using sasl::semantic::type_info_si;
using sasl::semantic::storage_si;

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

SASL_VISIT_DEF( binary_expression ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.left_expr );
	visit_child( child_ctxt, child_ctxt_init, v.right_expr );

	if( v.op == operators::assign ){
		bin_assign( v, data );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

SASL_VISIT_DEF( member_expression ){

	any child_ctxt = *data;
	sc_ptr(child_ctxt)->clear_data();

	visit_child( child_ctxt, v.expr );

	// Struct symbol
	type_info_si* tisi = dynamic_cast<type_info_si*>( v.expr->semantic_info().get() );
	shared_ptr<symbol> struct_sym = tisi->type_info()->symbol();
	shared_ptr<symbol> mem_sym = struct_sym->find_this( v.member->str );

	assert( mem_sym );

	cgllvm_sctxt* mem_ctxt = node_ctxt( mem_sym->node(), true );
	cgllvm_sctxt* agg_ctxt = node_ctxt( v.expr );

	assert( agg_ctxt );

	sc_ptr(data)->data( mem_ctxt );
	sc_data_ptr(data)->agg.parent = agg_ctxt;

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_expression ){
	shared_ptr<symbol> declsym = sc_env_ptr(data)->sym.lock()->find( v.var_name->str );
	assert( declsym && declsym->node() );

	sc_ptr(data)->storage_and_type( node_ctxt( declsym->node() ) );
	sc_data_ptr(data)->hint_name = v.var_name->str.c_str();
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( builtin_type ){

	shared_ptr<type_info_si> tisi = extract_semantic_info<type_info_si>( v );

	cgllvm_sctxt* pctxt = node_ctxt( tisi->type_info(), true );
	if ( !pctxt->data().val_type ){
		bool sign = false;
		Type const* ptype = llvm_type(v.value_typecode, sign);

		std::string tips = v.value_typecode.name() + std::string(" was not supported yet.");
		EFLIB_ASSERT_AND_IF( ptype, tips.c_str() ){
			return;
		}

		pctxt->data().val_type = ptype;
		pctxt->data().is_signed = sign;
	}
	sc_ptr( data )->data( pctxt );

	return;
}

// Generate normal function code.
SASL_VISIT_DEF( function_type ){
	sc_env_ptr(data)->sym = v.symbol();

	cgllvm_sctxt* fnctxt = node_ctxt(v.symbol()->node(), true);
	if( !fnctxt->data().self_fn ){
		create_fnsig( v, data );
	}
	if ( v.body ){
		sc_env_ptr(data)->parent_fn = sc_data_ptr(data)->self_fn;
		create_fnargs( v, data );
		create_fnbody( v, data );
	}

	// Here use the definition node.
	node_ctxt(v.symbol()->node(), true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( struct_type ){
	// Create context.
	// Declarator visiting need parent information.
	cgllvm_sctxt* ctxt = node_ctxt(v, true);

	// A struct is visited at definition type.
	// If the visited again, it must be as an alias_type.
	// So return environment directly.
	if( ctxt->data().val_type ){
		sc_ptr(data)->data(ctxt);
		return;
	}

	std::string name = v.symbol()->mangled_name();

	// Init data.
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	sc_env_ptr(&child_ctxt_init)->parent_struct = ctxt;

	any child_ctxt;

	// Visit children.
	// Add type of child into member types, and calculate index.
	vector<Type const*> members;
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child( child_ctxt, child_ctxt_init, decl );

		assert(
			sc_data_ptr(&child_ctxt)->declarator_count != 0
			&& sc_data_ptr(&child_ctxt)->val_type != NULL
			);

		members.insert(
			members.end(),
			sc_data_ptr(&child_ctxt)->declarator_count,
			sc_data_ptr(&child_ctxt)->val_type
			);
	}

	// Create
	StructType* stype = StructType::get( llcontext(), members, true );
	
	llmodule()->addTypeName( name.c_str(), stype );
	sc_data_ptr(data)->val_type = stype;

	ctxt->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( declarator ){

	// local *OR* member.
	// TODO TBD: Support member function and nested structure ?
	assert( !(sc_env_ptr(data)->parent_fn && sc_env_ptr(data)->parent_struct) );

	if( sc_env_ptr(data)->parent_fn ){
		visit_local_declarator( v, data );
	} else if( sc_env_ptr(data)->parent_struct ){
		visit_member_declarator( v, data );
	} else {
		visit_global_declarator(v, data);
	}
}

SASL_VISIT_DEF( variable_declaration ){
	// Visit type info
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.type_info );
	Type const* val_type = sc_data_ptr(&child_ctxt)->val_type;
	sc_env_ptr(&child_ctxt_init)->declarator_type = val_type;

	BOOST_FOREACH( shared_ptr<declarator> const& dclr, v.declarators ){
		visit_child( child_ctxt, child_ctxt_init, dclr );
	}

	sc_data_ptr(data)->declarator_count = v.declarators.size();

	sc_data_ptr(data)->val_type = val_type;
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( declaration_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.decl );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( compound_statement ){
	sc_env_ptr(data)->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child( child_ctxt, child_ctxt_init, *it );
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( jump_statement ){

	any child_ctxt_init = *data;
	any child_ctxt;

	if (v.jump_expr){
		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );
	}

	if ( v.code == jump_mode::_return ){
		return_statement(v, data);
	} else if ( v.code == jump_mode::_continue ){
		assert( sc_env_ptr(data)->continue_to );
		builder()->CreateBr( sc_env_ptr(data)->continue_to );
	} else if ( v.code == jump_mode::_break ){
		assert( sc_env_ptr(data)->break_to );
		builder()->CreateBr( sc_env_ptr(data)->break_to );
	}

	// Restart a new block for sealing the old block.
	restart_block(data, "");

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( return_statement, jump_statement ){
	if ( !v.jump_expr ){
		sc_data_ptr(data)->return_inst = builder()->CreateRetVoid();
	} else {
		sc_data_ptr(data)->return_inst = builder()->CreateRet( load( node_ctxt(v.jump_expr) ) );
	}
}

SASL_VISIT_DEF( expression_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.expr );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program ){
	mod_ptr()->create_module( v.name );

	ctxt_getter = boost::bind( &cgllvm_sisd::node_ctxt<node>, this, _1, false );
	boost::function<Value* (cgllvm_sctxt*)> loader
		= boost::bind( static_cast< Value* (cgllvm_sisd::*) (cgllvm_sctxt*)>( &cgllvm_sisd::load ), this, _1 );
	boost::function<void(Value*, cgllvm_sctxt*)> storer
		= boost::bind( static_cast<void (cgllvm_sisd::*) (Value*, cgllvm_sctxt*)>( &cgllvm_sisd::store ), this, _1, _2 );

	typeconv = create_type_converter( mod_ptr()->builder(), ctxt_getter, loader, storer );
	register_builtin_typeconv( typeconv, msi->type_manager() );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	// Generate return types.
	visit_child( child_ctxt, child_ctxt_init, v.retval_type );
	Type const* ret_type = sc_data_ptr(&child_ctxt)->val_type;
	EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){ return; }

	// Generate paramenter types.
	vector<Type const*> param_types;
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params ){
		visit_child( child_ctxt, child_ctxt_init, par );
		Type const* par_lltype = sc_data_ptr(&child_ctxt)->val_type;
		if ( par_lltype ){
			param_types.push_back( par_lltype );
		} else {
			EFLIB_ASSERT_AND_IF( ret_type, "Error occurs while parameter parsing." ){ return; }
		}
	}

	// Create function.
	FunctionType* ftype = FunctionType::get( ret_type, param_types, false );
	sc_data_ptr(data)->val_type = ftype;

	Function* fn = Function::Create( ftype, Function::ExternalLinkage, v.name->str, llmodule() );
	sc_data_ptr(data)->self_fn = fn;
}

SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type ){
	Function* fn = sc_data_ptr(data)->self_fn;

	// Register arguments names.
	Function::arg_iterator arg_it = fn->arg_begin();
	for( size_t arg_idx = 0; arg_idx < fn->arg_size(); ++arg_idx, ++arg_it){
		shared_ptr<parameter> par = v.params[arg_idx];
		sctxt_handle par_ctxt = node_ctxt( par );

		arg_it->setName( par->symbol()->unmangled_name() );
		par_ctxt->data().val = arg_it;
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnbody, function_type ){
	Function* fn = sc_data_ptr(data)->self_fn;

	any child_ctxt_init = *data;
	sc_env_ptr(&child_ctxt_init)->parent_fn = fn;

	any child_ctxt;

	// Create function body.
	// Create block
	restart_block( &child_ctxt_init, std::string(".entry") );
	restart_block( &child_ctxt_init, std::string(".body") );
	visit_child( child_ctxt, child_ctxt_init, v.body );
	clear_empty_blocks( fn );
}

SASL_SPECIFIC_VISIT_DEF( visit_member_declarator, declarator ){
	Type const* lltype = sc_env_ptr(data)->declarator_type;
	assert(lltype);

	// Needn't process init expression now.
	storage_si* si = dynamic_cast<storage_si*>( v.semantic_info().get() );
	sc_data_ptr(data)->agg.index = si->mem_index();
	sc_data_ptr(data)->val_type = lltype;

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( visit_global_declarator, declarator ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_SPECIFIC_VISIT_DEF( visit_local_declarator, declarator ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	sc_data_ptr(data)->val_type = sc_env_ptr(data)->declarator_type;
	create_alloca( sc_ptr(data), v.name->str );

	if ( v.init ){
		sc_env_ptr(&child_ctxt_init)->variable_to_fill = v.handle();
		visit_child( child_ctxt, child_ctxt_init, v.init );
		store( load(&child_ctxt), data );
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( bin_assign, binary_expression ){
	
	// Evaluated by visit(binary_expression)
	cgllvm_sctxt* lctxt = node_ctxt( v.left_expr );
	cgllvm_sctxt* rctxt = node_ctxt( v.right_expr );

	store( load(rctxt), lctxt );

	cgllvm_sctxt* pctxt = node_ctxt(v, true);
	pctxt->data( lctxt->data() );
	pctxt->env( sc_ptr(data) );
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

void cgllvm_sisd::restart_block( boost::any* data, std::string const& name ){
	assert( sc_env_ptr(data)->parent_fn );
	BasicBlock* restart = BasicBlock::Create( llcontext(), name, sc_env_ptr(data)->parent_fn );
	sc_env_ptr(data)->block = restart;
	builder()->SetInsertPoint(restart);
}

cgllvm_modimpl* cgllvm_sisd::mod_ptr(){
	assert( dynamic_cast<cgllvm_modimpl*>( mod.get() ) );
	return static_cast<cgllvm_modimpl*>( mod.get() );
}

llvm::Value* cgllvm_sisd::load( boost::any* data ){
	return load( sc_ptr(data) );
}

llvm::Value* cgllvm_sisd::load( cgllvm_sctxt* data ){
	assert(data);
	Value* val = data->data().val;
	
	const char* name = data->data().hint_name;
	name = ( name == NULL ? "" : name );

	do{
		if( val ){ break; }
		if( data->data().local ){
			val = builder()->CreateLoad( data->data().local, name );
			break;
		}
		if( data->data().global ){
			val = builder()->CreateLoad( data->data().global, name );
			break;
		}
		if( data->data().agg.parent ){
			val = load( data->data().agg.parent );
			val = builder()->CreateExtractValue( val, data->data().agg.index );
			break;
		}
		
		assert(!"Here is an invalid path!");
	} while(0);

	if( data->data().is_ref ){
		val = builder()->CreateLoad( val, name );
	}

	return val;
}

llvm::Value* cgllvm_sisd::load_ptr( cgllvm_sctxt* data ){
	cgllvm_sctxt_data* inner_data = &data->data();

	Value* addr = NULL;
	if( inner_data->val ){ addr = NULL; }
	if( inner_data->local ){
		addr = inner_data->local;
	}
	if( inner_data->global ){
		addr = inner_data->global;
	}
	if( inner_data->agg.parent ){
		Value* indexes[2];
		indexes[0] = ConstantInt::get( Type::getInt32Ty(llcontext()), 0 );
		indexes[1] = ConstantInt::get( Type::getInt32Ty(llcontext()), inner_data->agg.index );
		addr = builder()->CreateGEP(
			load_ptr( inner_data->agg.parent ), indexes, indexes + 2
			);
	}

	if( inner_data->is_ref ){
		if( !addr ){
			addr = inner_data->val;
		} else {
			const char* name = data->data().hint_name;
			addr = builder()->CreateLoad( addr, Twine( name ? name : "" ) );
		}
	}

	return addr;
}

void cgllvm_sisd::store( llvm::Value* v, boost::any* data ){
	store(v, sc_ptr(data) );
}

void cgllvm_sisd::store( llvm::Value* v, cgllvm_sctxt* data ){
	Value* addr = load_ptr( data );
	if( addr ){
		builder()->CreateStore( v, addr );
	} else {

		assert( data->data().agg.parent == NULL );
		assert( data->data().val == NULL );
		assert( data->data().local == NULL );
		assert( data->data().global == NULL );

		data->data().val = v;
	}
}

void cgllvm_sisd::create_alloca( cgllvm_sctxt* ctxt, std::string const& name ){
	assert( ctxt );
	assert( ctxt->data().val_type );

	Function* parent_fn = ctxt->env().parent_fn;
	if( parent_fn ){
		ctxt->data().local
			= builder()->CreateAlloca( ctxt->data().val_type, 0, name.c_str() );
	} else {
		ctxt->data().global
			 = cast<GlobalVariable>( llmodule()->getOrInsertGlobal( name, ctxt->data().val_type ) );
	}
}

void cgllvm_sisd::clear_empty_blocks( llvm::Function* fn )
{
	// Inner empty block, insert an br instruction for jumping to next block.
	// And the tail empty block we add an virtual return instruction.
	for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin();
		it != fn->getBasicBlockList().end(); ++it
		)
	{
		if( it->empty() ){
			Function::BasicBlockListType::iterator next_it = it;
			++next_it;

			builder()->SetInsertPoint( &(*it) );

			if( next_it != fn->getBasicBlockList().end() ){	
				mod_ptr()->builder()->CreateBr( &(*next_it) );
			} else {
				if( !fn->getReturnType()->isVoidTy() ){

					Value* val = zero_value( fn->getReturnType() );
					builder()->CreateRet(val);
				} else {
					builder()->CreateRetVoid();
				}
			}
		}
	}
}


END_NS_SASL_CODE_GENERATOR();