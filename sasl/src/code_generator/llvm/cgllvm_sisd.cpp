#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>

#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>
#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>

#include <sasl/enums/enums_utility.h>

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
#include <boost/format.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/scope_exit.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace llvm;
using namespace sasl::syntax_tree;
using namespace boost::assign;
using namespace sasl::utility;

using sasl::semantic::extract_semantic_info;
using sasl::semantic::symbol;
using sasl::semantic::type_info_si;
using sasl::semantic::storage_si;
using sasl::semantic::const_value_si;
using sasl::semantic::call_si;
using sasl::semantic::fnvar_si;
using sasl::semantic::operator_name;

using boost::addressof;
using boost::any_cast;
using boost::bind;

using std::vector;
using std::string;

#define SASL_VISITOR_TYPE_NAME cgllvm_sisd

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_sisd::~cgllvm_sisd(){
}

void cgllvm_sisd::mask_to_indexes( char indexes[4], uint32_t mask ){
	for( int i = 0; i < 4; ++i ){
		// XYZW is 1,2,3,4 but LLVM used 0,1,2,3
		char comp_index = static_cast<char>( (mask >> i*8) & 0xFF );
		if( comp_index == 0 ){
			indexes[i] = -1;
			break;
		}
		indexes[i] = comp_index - 1;
	}
}

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
		shared_ptr<type_info_si> larg_tsi = extract_semantic_info<type_info_si>(v.left_expr);
		shared_ptr<type_info_si> rarg_tsi = extract_semantic_info<type_info_si>(v.right_expr);

		//////////////////////////////////////////////////////////////////////////
		// type conversation for matching the operator prototype

		// get an overloadable prototype.
		std::vector< shared_ptr<expression> > args;
		args += v.left_expr, v.right_expr;

		symbol::overloads_t overloads
			= sc_env_ptr(data)->sym.lock()->find_overloads( operator_name( v.op ), typeconv, args );

		EFLIB_ASSERT_AND_IF( !overloads.empty(), "Error report: no prototype could match the expression." ){
			return;
		}
		EFLIB_ASSERT_AND_IF( overloads.size() == 1, "Error report: prototype was ambigous." ){
			return;
		}

		boost::shared_ptr<function_type> op_proto = overloads[0]->node()->as_handle<function_type>();

		shared_ptr<type_info_si> p0_tsi = extract_semantic_info<type_info_si>( op_proto->params[0] );
		shared_ptr<type_info_si> p1_tsi = extract_semantic_info<type_info_si>( op_proto->params[1] );

		// convert value type to match proto type.
		if( p0_tsi->entry_id() != larg_tsi->entry_id() ){
			if( ! node_ctxt( p0_tsi->type_info() ) ){
				visit_child( child_ctxt, child_ctxt_init, op_proto->params[0]->param_type );
			}

			node_ctxt(v.left_expr)->data().tyinfo
				= node_ctxt( p0_tsi->type_info() )->data().tyinfo;

			typeconv->convert( p0_tsi->type_info(), v.left_expr );
		}
		if( p1_tsi->entry_id() != rarg_tsi->entry_id() ){
			if( ! node_ctxt( p1_tsi->type_info() ) ){
				visit_child( child_ctxt, child_ctxt_init, op_proto->params[1]->param_type );
			}

			node_ctxt(v.left_expr)->data().tyinfo
				= node_ctxt( p0_tsi->type_info() )->data().tyinfo;

			typeconv->convert( p1_tsi->type_info(), v.right_expr );
		}

		// use type-converted value to generate code.

		value_t lval = node_ctxt(v.left_expr)->get_rvalue();
		value_t rval = node_ctxt(v.right_expr)->get_rvalue();

		value_t retval = null_value( NULL );

		if( lval.get_llvm_value() && rval.get_llvm_value() ){

			builtin_types lbtc = p0_tsi->type_info()->tycode;
			builtin_types rbtc = p1_tsi->type_info()->tycode;

			assert( lbtc == rbtc );

			if( is_scalar(lbtc) ){
				if( v.op == operators::add ){
					retval = lval + rval;
				} else {
					EFLIB_ASSERT_UNIMPLEMENTED();
				}
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED();
			}

			//if (v.op == operators::add){
			//	if( is_real(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateFAdd( lval, rval, "" );
			//	} else if( is_integer(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateAdd( lval, rval, "" );
			//	}
			//} else if ( v.op == operators::sub ){
			//	if( is_real(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateFSub( lval, rval, "" );
			//	} else if( is_integer(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateSub( lval, rval, "" );
			//	}
			//} else if ( v.op == operators::mul ){
			//	if( is_real(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateFMul( lval, rval, "" );
			//	} else if( is_integer(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateMul( lval, rval, "" );
			//	}
			//} else if ( v.op == operators::div ){
			//	if( is_real(lbtc) ){
			//		retval = mod_ptr()->builder()->CreateFDiv( lval, rval, "" );
			//	} else if( is_integer(lbtc) ){
			//		// TODO support signed integer yet.
			//		retval = mod_ptr()->builder()->CreateSDiv( lval, rval, "" );
			//	}
			//} else if ( v.op == operators::less ){
			//	if(is_real(lbtc)){
			//		retval = mod_ptr()->builder()->CreateFCmpULT( lval, rval );
			//	} else if ( is_integer(lbtc) ){
			//		if( is_signed(lbtc) ){
			//			retval = mod_ptr()->builder()->CreateICmpSLT(lval, rval);
			//		}
			//		if( is_unsigned(lbtc) ){
			//			retval = mod_ptr()->builder()->CreateICmpULT(lval, rval);
			//		}
			//	}
			//} else {
			//	EFLIB_INTERRUPT( (boost::format("Operator %s is not supported yet.") % v.op.name() ).str().c_str() );
			//}
		}

		store( sc_ptr(data)->get_value(), retval );
		node_ctxt(v, true)->copy( sc_ptr(data) );
	}
}

SASL_VISIT_DEF( member_expression ){

	any child_ctxt = *data;
	sc_ptr(child_ctxt)->clear_data();

	visit_child( child_ctxt, v.expr );

	cgllvm_sctxt* agg_ctxt = node_ctxt( v.expr );
	cgllvm_sctxt* mem_ctxt = NULL;

	assert( agg_ctxt );

	// Aggregated value
	type_info_si* tisi = dynamic_cast<type_info_si*>( v.expr->semantic_info().get() );

	value_t::kinds k = value_t::kind_unknown;
	if( tisi->type_info()->is_builtin() ){
		// Swizzle or write mask
		storage_si* mem_ssi = v.si_ptr<storage_si>();

		value_t vec_value = agg_ctxt->get_rvalue();
		store(
			sc_ptr(data)->get_value(),
			vec_value.swizzle( mem_ssi->swizzle() )
			);
	} else {
		// Member
		shared_ptr<symbol> struct_sym = tisi->type_info()->symbol();
		shared_ptr<symbol> mem_sym = struct_sym->find_this( v.member->str );

		assert( mem_sym );
		mem_ctxt = node_ctxt( mem_sym->node(), true );
		sc_ptr(data)->data( mem_ctxt );
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	// sc_data_ptr(data)->get_value().set_parent( &( agg_ctxt->get_value() ) )

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( constant_expression ){

	any child_ctxt_init = *data;
	any child_ctxt;

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	if( ! node_ctxt( c_si->type_info() ) ){
		visit_child( child_ctxt, child_ctxt_init, c_si->type_info() );
	}

	Value* retval = NULL;

	cgllvm_sctxt* const_ctxt = node_ctxt( c_si->type_info() );
	EFLIB_ASSERT_UNIMPLEMENTED();
	// Type const* const_lltype = const_ctxt->data().val_type;




	/*if( c_si->value_type() == builtin_types::_sint32 ){
		retval = ConstantInt::get( const_lltype, uint64_t( c_si->value<int32_t>() ), true );
	} else if ( c_si->value_type() == builtin_types::_uint32 ) {
		retval = ConstantInt::get( const_lltype, uint64_t( c_si->value<uint32_t>() ), false );
	} else if ( c_si->value_type() == builtin_types::_float ) {
		retval = ConstantFP::get( const_lltype, c_si->value<double>() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}*/

	// store( retval, sc_ptr(data) );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( call_expression ){
	any child_ctxt_init = *data;
	sc_ptr(&child_ctxt_init)->clear_data();

	any child_ctxt;

	Value* ret = NULL;

	call_si* csi = v.si_ptr<call_si>();
	if( csi->is_function_pointer() ){
		visit_child( child_ctxt, child_ctxt_init, v.expr );
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		// Get LLVM Function
		symbol* fn_sym = csi->overloaded_function();
		cgllvm_sctxt* fn_ctxt = node_ctxt( fn_sym->node(), false );
		// Function* fn = fn_ctxt->data().self_fn;
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
	//	sc_data_ptr(data)->val_type = fn_ctxt->data().val_type;

	//	vector<Value*> args;
	//	BOOST_FOREACH( shared_ptr<expression> const& arg_expr, v.args ){
	//		visit_child( child_ctxt, child_ctxt_init, arg_expr );

	//		type_info_si* arg_tisi = arg_expr->si_ptr<type_info_si>();

	//		llvm::Value* evaluated_value = load( &child_ctxt );
	//		builtin_types arg_ty = arg_tisi->type_info()->tycode;
	//		if( is_vector(arg_ty) ){
	//			args.push_back( llagg( cgv_vector( evaluated_value, ext.get() ) ).val );
	//		} else if( is_matrix( arg_ty ) ) {
	//			bool as_vec(false), as_mat(false);
	//			llvm::Type const* mat_abity = llvm_type( arg_ty, as_vec, as_mat );
	//			llagg ret = llagg( ext->null_value(mat_abity).val, ext.get() ) ;
	//			llagg matv = llagg( evaluated_value, ext.get() );
	//			for( size_t i = 0; i < matv.size(); ++i ){
	//				ret.set( i, llagg( cgv_vector( matv[i].val, ext.get() ) ) );
	//			}
	//			args.push_back(ret.val);
	//		} else {
	//			args.push_back(evaluated_value);
	//		}

	//	}

	//	// Create Call
	//	ret = builder()->CreateCall( fn, args.begin(), args.end() );
	//}

	//sc_data_ptr(data)->val = ret;
	//node_ctxt( v, true )->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_expression ){
	shared_ptr<symbol> declsym = sc_env_ptr(data)->sym.lock()->find( v.var_name->str );
	assert( declsym && declsym->node() );

	EFLIB_ASSERT_UNIMPLEMENTED();

	//sc_ptr(data)->get_value() == node_ctxt( declsym->node() )->value();
	//sc_ptr(data)->get_tysp() == node_ctxt( declsym->node() )->get_tysp();
	// sc_data_ptr(data)->hint_name = v.var_name->str.c_str();
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( builtin_type ){

	shared_ptr<type_info_si> tisi = extract_semantic_info<type_info_si>( v );

	cgllvm_sctxt* pctxt = node_ctxt( tisi->type_info(), true );

	//if ( !pctxt->data().val_type ){
	//	
	//	llvm_type( v.tycode, pctxt );
	//	
	//	assert( pctxt->data().val_type );
	//	std::string tips = v.tycode.name() + std::string(" was not supported yet.");
	//	EFLIB_ASSERT_AND_IF( pctxt->data().val_type, tips.c_str() ){
	//		return;
	//	}
	//}

	EFLIB_ASSERT_UNIMPLEMENTED();
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
	//if( ctxt->data().val_type ){
	//	sc_ptr(data)->data(ctxt);
	//	return;
	//}

	EFLIB_ASSERT_UNIMPLEMENTED();
	//std::string name = v.symbol()->mangled_name();

	//// Init data.
	//any child_ctxt_init = *data;
	//sc_ptr(child_ctxt_init)->clear_data();
	//sc_env_ptr(&child_ctxt_init)->parent_struct = ctxt;

	//any child_ctxt;

	//// Visit children.
	//// Add type of child into member types, and calculate index.
	//vector<Type const*> members;
	//BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
	//	visit_child( child_ctxt, child_ctxt_init, decl );

	//	assert(
	//		sc_data_ptr(&child_ctxt)->declarator_count != 0
	//		&& sc_data_ptr(&child_ctxt)->val_type != NULL
	//		);

	//	members.insert(
	//		members.end(),
	//		sc_data_ptr(&child_ctxt)->declarator_count,
	//		sc_data_ptr(&child_ctxt)->val_type
	//		);
	//}

	//// Create
	//StructType* stype = StructType::get( llcontext(), members, true );

	//llmodule()->addTypeName( name.c_str(), stype );
	//sc_data_ptr(data)->val_type = stype;

	//ctxt->copy( sc_ptr(data) );
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
	
	EFLIB_ASSERT_UNIMPLEMENTED();

	/*Type const* val_type = sc_data_ptr(&child_ctxt)->val_type;
	bool as_vector = sc_data_ptr(&child_ctxt)->as_vector;
	bool is_mat = sc_data_ptr(&child_ctxt)->is_matrix;

	sc_env_ptr(&child_ctxt_init)->declarator_type = val_type;
	sc_env_ptr(&child_ctxt_init)->is_mat = is_mat;
	sc_env_ptr(&child_ctxt_init)->as_vec = as_vector;

	BOOST_FOREACH( shared_ptr<declarator> const& dclr, v.declarators ){
		visit_child( child_ctxt, child_ctxt_init, dclr );
	}

	sc_data_ptr(data)->declarator_count = static_cast<int>( v.declarators.size() );

	sc_data_ptr(data)->val_type = val_type;
	node_ctxt(v, true)->copy( sc_ptr(data) );*/
}

SASL_VISIT_DEF( parameter ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.param_type );

	if( v.init ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	// sc_data_ptr(data)->val_type = sc_data_ptr( &child_ctxt )->val_type;
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
		EFLIB_ASSERT_UNIMPLEMENTED();
		//assert( sc_env_ptr(data)->continue_to );
		//builder()->CreateBr( sc_env_ptr(data)->continue_to );

	} else if ( v.code == jump_mode::_break ){
		EFLIB_ASSERT_UNIMPLEMENTED();

		//assert( sc_env_ptr(data)->break_to );
		//builder()->CreateBr( sc_env_ptr(data)->break_to );
	}

	// Restart a new block for sealing the old block.
	restart_block(data, "");

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( return_statement, jump_statement ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	if ( !v.jump_expr ){
		builder()->CreateRetVoid();
	} else {
		// builder()->CreateRet( load( node_ctxt(v.jump_expr) ) );
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

	typeconv = create_type_converter( ctxt_getter, this );
	register_builtin_typeconv( typeconv, msi->type_manager() );

	// Instrinsics will be generated before code was 
	process_intrinsics( v, data );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	EFLIB_ASSERT_UNIMPLEMENTED();

	// Generate return types.
	//visit_child( child_ctxt, child_ctxt_init, v.retval_type );
	//Type const* ret_type = sc_data_ptr(&child_ctxt)->val_type;
	//EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){ return; }

	//// Generate paramenter types.
	//vector<Type const*> param_types;
	//BOOST_FOREACH( shared_ptr<parameter> const& par, v.params ){
	//	visit_child( child_ctxt, child_ctxt_init, par );
	//	Type const* par_lltype = sc_data_ptr(&child_ctxt)->val_type;
	//	if ( par_lltype ){
	//		param_types.push_back( par_lltype );
	//	} else {
	//		EFLIB_ASSERT_AND_IF( ret_type, "Error occurs while parameter parsing." ){ return; }
	//	}
	//}

	//// Create function.
	//FunctionType* ftype = FunctionType::get( ret_type, param_types, false );
	//sc_data_ptr(data)->val_type = ftype;

	//Function* fn = Function::Create( ftype, Function::ExternalLinkage, v.symbol()->mangled_name(), llmodule() );
	//fn->setCallingConv( CallingConv::Fast );
	//sc_data_ptr(data)->self_fn = fn;
}

SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type ){
	push_fn( sc_data_ptr(data)->self_fn );
	scope_guard<void> pop_fn_on_exit( bind( &cg_service::pop_fn, this ) );

	EFLIB_ASSERT_UNIMPLEMENTED();

	// Register arguments names.
	assert( fn().arg_size() == v.params.size() );

	vector<string> param_names;
	size_t i_arg = 0;
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params )
	{
		sctxt_handle par_ctxt = node_ctxt( par );
		param_names.push_back( par->symbol()->unmangled_name() );
		par_ctxt->get_value() = fn().arg( i_arg++ );
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnbody, function_type ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	//Function* fn = sc_data_ptr(data)->self_fn;

	//any child_ctxt_init = *data;
	//// sc_env_ptr(&child_ctxt_init)->parent_fn = fn;

	//any child_ctxt;

	//// Create function body.
	//// Create block
	//restart_block( &child_ctxt_init, std::string(".entry") );
	//restart_block( &child_ctxt_init, std::string(".body") );
	//visit_child( child_ctxt, child_ctxt_init, v.body );
	//clear_empty_blocks( fn );
}

SASL_SPECIFIC_VISIT_DEF( visit_member_declarator, declarator ){
	// Type const* lltype = sc_env_ptr(data)->declarator_type;
	// assert(lltype);

	EFLIB_ASSERT_UNIMPLEMENTED();

	// Needn't process init expression now.
	//storage_si* si = dynamic_cast<storage_si*>( v.semantic_info().get() );
	//sc_data_ptr(data)->agg.index = si->mem_index();
	//sc_data_ptr(data)->val_type = lltype;
	//sc_data_ptr(data)->is_matrix = sc_env_ptr(data)->is_mat;
	//sc_data_ptr(data)->as_vector = sc_env_ptr(data)->as_vec;

	//node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_SPECIFIC_VISIT_DEF( visit_global_declarator, declarator ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_SPECIFIC_VISIT_DEF( visit_local_declarator, declarator ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	EFLIB_ASSERT_UNIMPLEMENTED();

	//sc_data_ptr(data)->val_type = sc_env_ptr(data)->declarator_type;
	//sc_data_ptr(data)->as_vector = sc_env_ptr(data)->as_vec;
	//sc_data_ptr(data)->is_matrix = sc_env_ptr(data)->is_mat;

	//create_alloca( sc_ptr(data), v.name->str );

	//if ( v.init ){
	//	sc_env_ptr(&child_ctxt_init)->variable_to_fill = v.handle();
	//	visit_child( child_ctxt, child_ctxt_init, v.init );
	//	store( load(&child_ctxt), data );
	//}

	//node_ctxt(v, true)->copy( sc_ptr(data) );
}

/* Make binary assignment code.
*    Note: Right argument is assignee, and left argument is value.
*/
SASL_SPECIFIC_VISIT_DEF( bin_assign, binary_expression ){

	EFLIB_ASSERT_UNIMPLEMENTED();

	//shared_ptr<type_info_si> larg_tsi = extract_semantic_info<type_info_si>(v.left_expr);
	//shared_ptr<type_info_si> rarg_tsi = extract_semantic_info<type_info_si>(v.right_expr);

	//if ( larg_tsi->entry_id() != rarg_tsi->entry_id() ){
	//	if( typeconv->implicit_convertible( rarg_tsi->entry_id(), larg_tsi->entry_id() ) ){
	//		typeconv->convert( rarg_tsi->type_info(), v.left_expr );
	//	} else {
	//		assert( !"Expression could not converted to storage type." );
	//	}
	//}

	//// Evaluated by visit(binary_expression)
	//cgllvm_sctxt* lctxt = node_ctxt( v.left_expr );
	//cgllvm_sctxt* rctxt = node_ctxt( v.right_expr );

	//store( load(lctxt), rctxt );

	//cgllvm_sctxt* pctxt = node_ctxt(v, true);
	//pctxt->data( rctxt->data() );
	//pctxt->env( sc_ptr(data) );
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

Constant* cgllvm_sisd::zero_value( boost::shared_ptr<tynode> typespec )
{
	//if( typespec->is_builtin() ){
	//	builtin_types btc = typespec->tycode;
	//	Type const* valtype = node_ctxt(typespec)->data().val_type;
	//	if( is_integer( btc ) ){
	//		return ConstantInt::get( valtype, 0, is_signed(btc) );
	//	}
	//	if( is_real( btc ) ){
	//		return ConstantFP::get( valtype, 0.0 );
	//	}
	//}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

cgllvm_sctxt* cgllvm_sisd::node_ctxt( sasl::syntax_tree::node& v, bool create_if_need /*= false */ )
{
	return cgllvm_impl::node_ctxt<cgllvm_sctxt>(v, create_if_need);
}

void cgllvm_sisd::restart_block( boost::any* data, std::string const& name ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	/*assert( sc_env_ptr(data)->parent_fn );
	BasicBlock* restart = BasicBlock::Create( llcontext(), name, sc_env_ptr(data)->parent_fn );
	sc_env_ptr(data)->block = restart;
	builder()->SetInsertPoint(restart);*/
}

cgllvm_modimpl* cgllvm_sisd::mod_ptr(){
	assert( dynamic_cast<cgllvm_modimpl*>( mod.get() ) );
	return static_cast<cgllvm_modimpl*>( mod.get() );
}

//llvm::Value* cgllvm_sisd::load( boost::any* data ){
//	return load( sc_ptr(data) );
//}
//
//llvm::Value* cgllvm_sisd::load( cgllvm_sctxt* data ){
//	assert(data);
//	Value* val = data->data().val;
//
//	const char* name = data->data().hint_name;
//	name = ( name == NULL ? "" : name );
//
//	do{
//		if( val ){ break; }
//		if( data->data().local ){
//			val = builder()->CreateLoad( data->data().local, name );
//			break;
//		}
//		if( data->data().global ){
//			val = builder()->CreateLoad( data->data().global, name );
//			break;
//		}
//		if( data->data().agg.parent ){
//			val = load( data->data().agg.parent );
//			if( data->data().agg.is_swizzle ){
//				// Swizzle with shuffle instruction.
//				char indices[4] = {-1, -1, -1, -1};
//				mask_to_indexes(indices, data->data().agg.swizzle);
//
//				cgv_vector v( NULL, ext.get() );
//				if( val->getType()->isVectorTy() ){
//					v = cgv_vector( val, ext.get() );
//				} else {
//					v = cgv_vector( llval( val, ext.get() ), 1 );
//				}
//
//				v = v.swizzle( indices, 4 );
//
//			} else {
//				llagg agg_val( val, ext.get() );
//				val = agg_val[data->data().agg.index].val;
//			}
//			break;
//		}
//
//		return NULL;
//	} while(0);
//
//	if( data->data().is_ref ){
//		val = builder()->CreateLoad( val, name );
//	}
//
//	if( data->data().as_vector && val->getType()->isStructTy() ){
//		return cgv_vector( val, ext.get() ).val;
//	}
//	return val;
//}

llvm::Value* cgllvm_sisd::to_abi( builtin_types hint, llvm::Value* v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
	//if( is_vector(hint) ){
	//	return cgv_vector( llagg( v, ext.get() ) ).val;
	//} else if( is_matrix( hint ) ) {
	//	bool as_vec(false), as_mat(false);
	//	llvm::Type const* mat_abity = llvm_type( hint, as_vec, as_mat );
	//	llagg ret = llagg( ext->null_value(mat_abity).val, ext.get() ) ;
	//	llagg matv = llagg( v, ext.get() );
	//	for( size_t i = 0; i < matv.size(); ++i ){
	//		ret.set( i, llagg( cgv_vector( matv[i].val, ext.get() ) ) );
	//	}
	//	return ret.val;
	//} else {
	//	return v;
	//}
}

llvm::Value* cgllvm_sisd::from_abi( builtin_types hint, llvm::Value* v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
	/*if( is_vector(hint) ){
		return llagg( cgv_vector( v, ext.get() ) ).val;
	} else if( is_matrix( hint ) ) {
		bool as_vec(false), as_mat(false);
		llvm::Type const* mat_abity = llvm_type( hint, as_vec, as_mat );
		llagg matv = llagg( ext->null_value(mat_abity).val, ext.get() ) ;
		llagg ret = llagg( v, ext.get() );
		for( size_t i = 0; i < matv.size(); ++i ){
			ret.set( i, llagg( cgv_vector( matv[i].val, ext.get() ) ) );
		}
		return ret.val;
	} else {
		return v;
	}*/
}

//llvm::Value* cgllvm_sisd::load_ptr( cgllvm_sctxt* data ){
//	cgllvm_sctxt_data* inner_data = &data->data();
//
//	Value* addr = NULL;
//	if( inner_data->val ){ addr = NULL; }
//	if( inner_data->local ){
//		addr = inner_data->local;
//	}
//	if( inner_data->global ){
//		addr = inner_data->global;
//	}
//	if( inner_data->agg.parent ){
//		Value* indexes[2];
//		indexes[0] = ConstantInt::get( Type::getInt32Ty(llcontext()), 0 );
//		indexes[1] = ConstantInt::get( Type::getInt32Ty(llcontext()), inner_data->agg.index );
//		addr = builder()->CreateGEP(
//			load_ptr( inner_data->agg.parent ), indexes, indexes + 2
//			);
//	}
//
//	if( inner_data->is_ref ){
//		if( !addr ){
//			addr = inner_data->val;
//		} else {
//			const char* name = data->data().hint_name;
//			addr = builder()->CreateLoad( addr, Twine( name ? name : "" ) );
//		}
//	}
//
//	return addr;
//}
//
//void cgllvm_sisd::store( llvm::Value* v, boost::any* data ){
//	store(v, sc_ptr(data) );
//}
//
//// Store will be enabled in following cases:
////	value to address of same type.
////	value to null data.
////	scalar to vector value.
//void cgllvm_sisd::store( llvm::Value* v, cgllvm_sctxt* data ){
//	if( !data->data().agg.is_swizzle ){
//		Value* addr = load_ptr( data );
//
//		if( addr ){
//			if( data->data().as_vector && !data->data().is_matrix ){
//				v = llagg( v, ext.get() ).val;
//			}
//
//			builder()->CreateStore( v, addr );
//		} else {
//			assert( data->data().agg.parent == NULL );
//			assert( data->data().val == NULL );
//			assert( data->data().local == NULL );
//			assert( data->data().global == NULL );
//
//			data->data().val = v;
//		}
//	} else {
//		// Write mask
//		// Steps:
//		//  Load or create vector value
//		//  Shuffle or insert value to vector.
//		//  Save new vector to parent.
//		char mask_indexes[4] = {-1, -1, -1, -1};
//		mask_to_indexes( mask_indexes, data->data().agg.swizzle );
//
//		cgv_vector vec( load(data->data().agg.parent), ext.get() );
//
//		if( v->getType()->isIntegerTy() || v->getType()->isFloatingPointTy() ){
//			// Scalar, insert directly
//			if( !vec.val ){
//				vec = cgv_vector( llval(v, ext.get()), 1 );
//			} else {
//				vec.set( 0, llval(v, ext.get()) );
//			}
//		} else {
//			// Vector, insert per element.
//			assert( v->getType()->isVectorTy() );
//			for( int i = 0; i < 4 && mask_indexes[i] != -1; ++i ){
//				cgv_vector src_vec( v, ext.get() );
//				vec.set( mask_indexes[i], src_vec[i] );
//			}
//		}
//
//		store( vec.val, data->data().agg.parent );
//	}
//}
//
//void cgllvm_sisd::create_alloca( cgllvm_sctxt* ctxt, std::string const& name ){
//	assert( ctxt );
//	assert( ctxt->data().val_type );
//
//	Function* parent_fn = ctxt->env().parent_fn;
//	if( parent_fn ){
//		ctxt->data().local
//			= builder()->CreateAlloca( ctxt->data().val_type, 0, name.c_str() );
//	} else {
//		ctxt->data().global
//			= cast<GlobalVariable>( llmodule()->getOrInsertGlobal( name, ctxt->data().val_type ) );
//	}
//}
//
void cgllvm_sisd::clear_empty_blocks( llvm::Function* fn )
{
	// Inner empty block, insert an br instruction for jumping to next block.
	// And the tail empty block we add an virtual return instruction.

	EFLIB_ASSERT_UNIMPLEMENTED();

	/*typedef Function::BasicBlockListType::iterator block_iterator_t;
	block_iterator_t beg = fn->getBasicBlockList().begin();
	block_iterator_t end = fn->getBasicBlockList().end();

	for(  block_iterator_t it = beg; it != end; ++it )
	{
		if( !it->getTerminator() ){
			block_iterator_t next_it = it;
			++next_it;

			builder()->SetInsertPoint( &(*it) );

			if( next_it != fn->getBasicBlockList().end() ){	
				mod_ptr()->builder()->CreateBr( &(*next_it) );
			} else {
				if( !fn->getReturnType()->isVoidTy() ){
					ext->return_( ext->null_value( fn->getReturnType() ) );
				} else {
					ext->return_();
				}
			}
		}
	}*/
}

template <typename ElementT> llvector<ElementT> cgllvm_sisd::mul_vm(
	llvm::Value* v, llvm::Value* m,
	size_t vec_size, size_t mat_vec_size,
	Type const* ret_type
	)
{
	llvector< ElementT > lval = llvector<ElementT>::create( ext.get(), v );
	llarray< llvector< ElementT > > rval( m, ext.get() );

	llvector<ElementT> ret_val = ext->null_value< llvector<ElementT> >(ret_type);

	for(size_t i = 0; i < mat_vec_size; ++i){
		ElementT agg_value = ret_val[i];
		for( size_t j = 0; j < vec_size; ++j ){
			agg_value = agg_value + lval[j] * rval[j][i];
		}
		ret_val.set( i, agg_value );
	}

	return ret_val;
}

template <typename ElementT> llvector<ElementT> cgllvm_sisd::mul_mv(
	llvm::Value* m, llvm::Value* v,
	size_t vec_size, size_t n_vec,
	llvm::Type const* ret_type )
{
	typedef llvector< ElementT > vector_t;
	typedef llarray< llvector< ElementT > > matrix_t;

	matrix_t lval( m, ext.get() );
	vector_t rval( v, ext.get() );

	vector_t ret_val = ext->null_value< vector_t >(ret_type);

	for(size_t i = 0; i < n_vec; ++i){
		ElementT agg_value = ret_val[i];
		for( size_t j = 0; j < vec_size; ++j ){
			agg_value = agg_value + lval[i][j] * rval[j];
		}
		ret_val.set( i, agg_value );
	}

	return ret_val;
}

template <typename ElementT> ElementT cgllvm_sisd::dot_prod(
	llvm::Value* lhs, llvm::Value* rhs,
	size_t vec_size,
	llvm::Type const* ret_type
	)
{
	typedef llvector<ElementT> vector_t;

	vector_t lval( lhs, ext.get() );
	vector_t rval( rhs, ext.get() );

	ElementT ret_val = ext->null_value<ElementT>(ret_type);
	for( size_t i = 0; i < vec_size; ++i ){
		ret_val = ret_val + lval[i] * rval[i];
	}
	
	return ret_val;
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

		push_fn( intrinsic_ctxt->data().self_fn );
		scope_guard<void> pop_fn_on_exit( bind( &cg_service::pop_fn, this ) );

		EFLIB_ASSERT_UNIMPLEMENTED();
		/*BasicBlock* body = BasicBlock::Create( llcontext(), ".body", fn );
		builder()->SetInsertPoint( body );*/

		if( intr->unmangled_name() == "mul" )
		{
			// Set Argument name
			fn().arg_name( 0, ".lhs" );
			fn().arg_name( 1, ".rhs" );

			// Get Type infos
			shared_ptr<tynode> lpar_type = intr_fn->params[0]->si_ptr<type_info_si>()->type_info();
			shared_ptr<tynode> rpar_type = intr_fn->params[1]->si_ptr<type_info_si>()->type_info();
			assert( lpar_type && rpar_type );
			builtin_types lbtc = lpar_type->tycode;
			builtin_types rbtc = rpar_type->tycode;

			shared_ptr<value_tyinfo> result_ty = fn().get_return_ty();

			EFLIB_ASSERT_UNIMPLEMENTED();
			// TODO need to be optimized.

			// vec_m mul(vec_n, mat_mxn);
			//if( is_vector( lbtc ) && is_matrix( rbtc ) ){
			//	if( scalar_of(lbtc) == builtin_types::_float ){
			//		emit_return(
			//			mul_vm<llfloat>( larg, rarg,
			//			vector_size(lbtc), vector_size(rbtc),
			//			ret_type )
			//			);
			//	} else if ( scalar_of(lbtc) == builtin_types::_sint32 ){
			//		ext->return_(
			//			mul_vm<lli32>( larg, rarg,
			//			vector_size(lbtc), vector_size(rbtc),
			//			ret_type )
			//			);
			//	} else {
			//		// EFLIB_ASSERT_UNIMPLEMENTED();
			//	}
			//} else if( is_matrix( lbtc ) && is_vector( rbtc ) ) {
			//	if( scalar_of(lbtc) == builtin_types::_float ){
			//		ext->return_(
			//			mul_mv<llfloat>( larg, rarg,
			//			vector_size(lbtc), vector_count(lbtc),
			//			ret_type )
			//			);
			//	}
			//} else {
			//	// EFLIB_ASSERT_UNIMPLEMENTED();
			//}
		}
		else if( intr->unmangled_name() == "dot" )
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
			// Set Argument name
			//Argument* larg = fn->getArgumentList().begin();
			//Argument* rarg = ++fn->getArgumentList().begin();

			//larg->setName( ".lhs" );
			//rarg->setName( ".rhs" );

			//// Get Type infos
			//shared_ptr<tynode> lpar_type = intr_fn->params[0]->si_ptr<type_info_si>()->type_info();
			//shared_ptr<tynode> rpar_type = intr_fn->params[1]->si_ptr<type_info_si>()->type_info();
			//assert( lpar_type && rpar_type );
			//builtin_types lbtc = lpar_type->tycode;
			//builtin_types rbtc = rpar_type->tycode;

			//Type const* ret_type = fn->getReturnType();

			//assert( is_vector(lbtc) && is_vector(rbtc) );
			//assert( scalar_of(lbtc) == scalar_of(rbtc) );
			//assert( vector_size(lbtc) == vector_size(rbtc) );

			//size_t vec_size = vector_size(lbtc);

			//if( scalar_of(lbtc) == builtin_types::_float )
			//{
			//	ext->return_(
			//		dot_prod<llfloat>( larg, rarg, vec_size, ret_type )
			//		);
			//} 
			//else if ( scalar_of(lbtc) == builtin_types::_sint32 )
			//{
			//	ext->return_(
			//		dot_prod<lli32>( larg, rarg, vec_size, ret_type )
			//		);
			//}
			//else 
			//{
			//	// EFLIB_ASSERT_UNIMPLEMENTED();
			//}
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}
}


END_NS_SASL_CODE_GENERATOR();