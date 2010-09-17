#include <sasl/include/code_generator/llvm/cgllvm_impl.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>

BEGIN_NS_SASL_CODE_GENERATOR();

#define UNIMPLEMENTED() assert(!"Unimplemented!");

using namespace std;
using namespace syntax_tree;
using namespace llvm;

using semantic::symbol;

typedef boost::shared_ptr<cgllvm_common_context> common_ctxt_handle;

//////////////////////////////////////////////////////////////////////////
// utility functions.

template< typename NodeT >
static common_ctxt_handle extract_common_ctxt( NodeT& v ){
	return extract_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle extract_common_ctxt( boost::shared_ptr<NodeT> v ){
	return extract_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle get_common_ctxt( NodeT& v ){
	return get_or_create_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle get_common_ctxt( boost::shared_ptr<NodeT> v ){
	return get_or_create_codegen_context<cgllvm_common_context>(v);
}

template<typename NodeT>
static common_ctxt_handle parent_ctxt( boost::shared_ptr<NodeT> v ){
	boost::shared_ptr<symbol> parent_sym = v->symbol()->parent();
	if( parent_sym ){
		return extract_codegen_context<cgllvm_common_context>( parent_sym->node() );
	}
	return common_ctxt_handle();
}
//////////////////////////////////////////////////////////////////////////
//
llvm_code_generator::llvm_code_generator( )
{
}

void llvm_code_generator::visit( unary_expression& ){
	
}

void llvm_code_generator::visit( cast_expression& ){}
void llvm_code_generator::visit( binary_expression& ){
	//// generate left and right expr.
	//v.left_expr->accept(this);
	//v.right_expr->accept(this);

	////
	//boost::shared_ptr<symbol> lsym = v.left_expr->symbol();
	//boost::shared_ptr<symbol> rsym = v.right_expr->symbol();

	//boost::shared_ptr<type_specifier> ltype = lsym->semantic_info<value_type_semantic_info>()->value_type();
	//boost::shared_ptr<type_specifier> rtype = rsym->semantic_info<value_type_semantic_info>()->value_type();

	//boost::shared_ptr<class semantic_info> ret_syminfo = v.symbol()->get_or_create_semantic_info<class semantic_info>();

	//// generate code
	//if (v.op == operators::add){
	//	if ( ltype->type_id_of_value == buildin_type_code::_sint32 &&
	//		ltype->type_id_of_value == buildin_type_code::_sint32
	//	){
	//		llvm::Value* ret_val = cg_ctxt.builder->CreateAdd( 
	//			lsym->semantic_info<class semantic_info>()->llvm_value,
	//			rsym->semantic_info<class semantic_info>()->llvm_value,
	//			generate_temporary_name()
	//			);
	//		ret_syminfo->llvm_value = ret_val;
	//	}
	//} else {
	//	throw "cannot support yet";
	//}
}
void llvm_code_generator::visit( expression_list& ){}
void llvm_code_generator::visit( cond_expression& ){}
void llvm_code_generator::visit( index_expression& ){}
void llvm_code_generator::visit( call_expression& ){}
void llvm_code_generator::visit( member_expression& ){}

void llvm_code_generator::visit( constant_expression& ){
	//using sasl::semantic_analyser::value_semantic_info;
	//boost::shared_ptr<value_semantic_info> val_syminfo
	//	= get_or_create_semantic_info<value_semantic_info>(v);
	//Value* ret_v = NULL;

	//if ( v.value->valtype == literal_constant_types::real ){
	//	if ( v.value->is_single() ){
	//		ret_v = ConstantFP::get( ctxt, APFloat( (float)val_syminfo->value<double>() ) );
	//	} else {
	//		ret_v = ConstantFP::get( ctxt, APFloat(val_syminfo->value<double>()) );
	//	}
	//} else if ( v.value->valtype == literal_constant_types::integer ){
	//	if ( v.value->is_unsigned() ){
	//		ret_v = ConstantInt::get( Type::getInt64Ty(ctxt), val_syminfo->value<unsigned long>() );
	//	} else {
	//		union {
	//			unsigned long u;
	//			signed long s;
	//		} u_to_s;
	//		u_to_s.s = val_syminfo->value<long>();
	//		ret_v = ConstantInt::get( Type::getInt64Ty(ctxt), u_to_s.u, true );
	//	}
	//}

	//boost::shared_ptr< class semantic_info > syminfo = get_or_create_semantic_info<class semantic_info>(v);
	//syminfo->llvm_value = ret_v;
}

void llvm_code_generator::visit( identifier& ){

}
void llvm_code_generator::visit( variable_expression& ){

}
// declaration & type specifier
void llvm_code_generator::visit( initializer& ){}
void llvm_code_generator::visit( expression_initializer& ){}
void llvm_code_generator::visit( member_initializer& ){}
void llvm_code_generator::visit( declaration& ){}
void llvm_code_generator::visit( variable_declaration& ){
	//using ::sasl::semantic::extract_semantic_info;
	//using ::sasl::semantic::variable_semantic_info;
	//using ::sasl::semantic::get_or_create_semantic_info;

	//v.type_info->accept( this );
	//boost::shared_ptr<variable_semantic_info> vsyminfo = extract_semantic_info<variable_semantic_info>(v);
	//boost::shared_ptr<llvm_semantic_info> lsyminfo = get_or_create_semantic_info<llvm_semantic_info>(v);
	//boost::shared_ptr<llvm_semantic_info> ltsyminfo = extract_semantic_info<llvm_semantic_info>(v.type_info);
	//
	////TODO: GET VALUE OF INITIALIZER

	//if ( vsyminfo->is_local() ){
	//	// TODO: GENERATE LOCAL VARIABLE	
	//} else {
	//	// generate global variable
	//	GlobalVariable* gv = cast<GlobalVariable>(ctxt->module()->getOrInsertGlobal( v.name->str, ltsyminfo->llvm_type ));
	//	// TODO: OTHER OPERATIONS. SUCH AS LINKAGE
	//	// ...
	//	lsyminfo->llvm_gvar = gv;
	//}
}

void llvm_code_generator::visit( type_definition& ){}
void llvm_code_generator::visit( type_specifier& ){
}
void llvm_code_generator::visit( buildin_type& v ){
	if ( v.codegen_ctxt() ){ return; }
	common_ctxt_handle type_ctxt = get_common_ctxt(v);
	if ( v.value_typecode == buildin_type_code::_void ){
		type_ctxt->type = llvm::Type::getVoidTy( ctxt->context() );
	} else if ( v.value_typecode == buildin_type_code::_sint8 ){
		type_ctxt->type = llvm::Type::getInt8Ty( ctxt->context() );
	} else {
		UNIMPLEMENTED();
	}
}
void llvm_code_generator::visit( array_type& ){}
void llvm_code_generator::visit( struct_type& ){}
void llvm_code_generator::visit( parameter& ){

}

void llvm_code_generator::visit( function_type& v ){

	// skip if context existed.
	if ( v.codegen_ctxt() ) { return; }
	common_ctxt_handle fctxt = get_common_ctxt( v );

	// Generate return types.
	v.retval_type->accept( this );
	const llvm::Type* ret_type = extract_common_ctxt(v.retval_type)->type;

	// Generate paramenter types.
	vector< const llvm::Type*> param_types;
	for( vector< boost::shared_ptr<parameter> >::iterator it = v.params.begin(); it != v.params.end(); ++it ){
		(*it)->accept(this);
		common_ctxt_handle par_ctxt = get_common_ctxt( (*it)->param_type );
		param_types.push_back( par_ctxt->type );
	}

	// Create function.
	fctxt->func_type = llvm::FunctionType::get( ret_type, param_types, false );
	fctxt->func = Function::Create( fctxt->func_type, Function::ExternalLinkage, v.name->str, ctxt->module().get() );

	// Register parameter names.
	llvm::Function::arg_iterator arg_it = fctxt->func->arg_begin();
	for( int arg_idx = 0; arg_idx < fctxt->func->arg_size(); ++arg_idx, ++arg_it){
		boost::shared_ptr<parameter> par = v.params[arg_idx];
		arg_it->setName( par->symbol()->name() );
		common_ctxt_handle par_ctxt = get_common_ctxt( par->param_type );
		par_ctxt->arg = boost::addressof( *arg_it );
	}

	// Create function body.
	if ( v.body ){
		v.body->accept( this );
	}
}

// statement
void llvm_code_generator::visit( statement& ){
}
void llvm_code_generator::visit( declaration_statement& ){}
void llvm_code_generator::visit( if_statement& ){}
void llvm_code_generator::visit( while_statement& ){}
void llvm_code_generator::visit( dowhile_statement& ){}
void llvm_code_generator::visit( case_label& ){}
void llvm_code_generator::visit( switch_statement& ){}
void llvm_code_generator::visit( compound_statement& ){
	// generate function code
	//BasicBlock* funcCodeBlock = BasicBlock::Create( ctxt, "entry", func );
	//cg_ctxt.builder->SetInsertPoint(funcCodeBlock);
	//for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
	//	it != v.stmts.end(); ++it ){
	//	(*it)->accept( this );
	//}
}
void llvm_code_generator::visit( expression_statement& ){}
void llvm_code_generator::visit( jump_statement& ){}

void llvm_code_generator::visit( ident_label& ){ }

void llvm_code_generator::visit( program& v ){
	if ( ctxt ){
		return;
	}

	ctxt = create_codegen_context<cgllvm_global_context>(v.handle());
	ctxt->create_module( v.name );

	for( vector< boost::shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it ){
		(*it)->accept( this );
	}
}

boost::shared_ptr<llvm_code> llvm_code_generator::generated_module(){
	return boost::shared_polymorphic_cast<llvm_code>(ctxt);
}

END_NS_SASL_CODE_GENERATOR();