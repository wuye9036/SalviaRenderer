#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <sasl/include/host/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using sasl::utility::to_builtin_types;
using salviar::sv_usage;
using salviar::su_none;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::storage_usage_count;
using salviar::sv_layout;
using llvm::Type;
using llvm::StructType;
using llvm::StructLayout;
using llvm::PointerType;
using llvm::FunctionType;
using llvm::Function;
using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_simd

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_simd::cgllvm_simd(): entry_fn(NULL){
	memset( entry_structs, 0, sizeof( entry_structs ) );
}

cgllvm_simd::~cgllvm_simd(){}

cg_service* cgllvm_simd::service() const{
	return const_cast<cgllvm_simd*>(this);
}

abis cgllvm_simd::local_abi( bool is_c_compatible ) const
{
	return abi_package;
}

void cgllvm_simd::create_entries()
{
	create_entry_param( su_stream_in );
	create_entry_param( su_stream_out );
	create_entry_param( su_buffer_in );
	create_entry_param( su_buffer_out );
}

void cgllvm_simd::create_entry_param( sv_usage usage )
{
	vector<sv_layout*> svls = abii->layouts(usage);
	vector<Type*>& tys = entry_tys[usage];

	BOOST_FOREACH( sv_layout* si, svls ){
		builtin_types storage_bt = to_builtin_types(si->value_type);

		entry_tyns[usage].push_back( storage_bt );
		
		if( su_stream_in == usage || su_stream_out == usage ){
			Type* storage_ty = type_( storage_bt, abi_package );
			tys.push_back( storage_ty );
		} else {
			Type* storage_ty = type_( storage_bt, abi_llvm );
			tys.push_back( storage_ty );
		}
	}

	char const* struct_name = NULL;
	switch( usage ){
	case su_stream_in:
		struct_name = ".s.stri";
		break;
	case su_buffer_in:
		struct_name = ".s.bufi";
		break;
	case su_stream_out:
		struct_name = ".s.stro";
		break;
	case su_buffer_out:
		struct_name = ".s.bufo";
		break;
	}
	assert( struct_name );

	// Tys must not be empty. So placeholder (int8) will be inserted if tys is empty.
	StructType* out_struct = tys.empty()
		? StructType::create( struct_name, type_(builtin_types::_sint8, abi_llvm), NULL )
		: StructType::create( tys, struct_name );

	entry_structs[usage] = out_struct;

	// Update Layout physical informations.
	StructLayout const* struct_layout = target_data->getStructLayout( out_struct );

	size_t next_offset = 0;
	for( size_t i_elem = 0; i_elem < svls.size(); ++i_elem ){
		size_t offset = next_offset;
		svls[i_elem]->offset = offset;
		svls[i_elem]->physical_index = i_elem;

		size_t next_i_elem = i_elem + 1;
		if( next_i_elem < tys.size() ){
			next_offset = struct_layout->getElementOffset( static_cast<unsigned>(next_i_elem) );
		} else {
			next_offset = struct_layout->getSizeInBytes();
		}
		
		svls[i_elem]->element_padding = (next_offset - offset) - svls[i_elem]->element_size;
	}
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( binary_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );
SASL_VISIT_DEF_UNIMPL( call_expression );
SASL_VISIT_DEF_UNIMPL( member_expression );
SASL_VISIT_DEF_UNIMPL( constant_expression );
SASL_VISIT_DEF_UNIMPL( variable_expression );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( expression_initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( alias_type );

// statement
SASL_VISIT_DEF_UNIMPL( statement );
SASL_VISIT_DEF_UNIMPL( declaration_statement );
SASL_VISIT_DEF_UNIMPL( if_statement );
SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( for_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( ident_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );
SASL_VISIT_DEF_UNIMPL( compound_statement );
SASL_VISIT_DEF_UNIMPL( expression_statement );
SASL_VISIT_DEF_UNIMPL( jump_statement );
SASL_VISIT_DEF_UNIMPL( labeled_statement );

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	parent_class::before_decls_visit( v, data );
	create_entries();
}

void add_type_ref( Type* ty, vector<Type*>& tys )
{
	tys.push_back( PointerType::getUnqual( ty ) );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type )
{
	if( !entry_fn && abii->is_entry( v.symbol() ) ){

		boost::any child_ctxt;

		vector<Type*> param_types;

		add_type_ref( entry_structs[su_stream_in], param_types );
		add_type_ref( entry_structs[su_buffer_in], param_types );
		add_type_ref( entry_structs[su_stream_out], param_types );
		add_type_ref( entry_structs[su_buffer_out], param_types );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cgllvm_impl::context() ), param_types, false );
		Function* fn = Function::Create( fntype, Function::ExternalLinkage, v.symbol()->mangled_name(), cgllvm_impl::module() );
		entry_fn = fn;
		// entry_sym = v.symbol().get();

		sc_data_ptr(data)->self_fn.fn = fn;
		sc_data_ptr(data)->self_fn.fnty = &v;
	} else {
		parent_class::create_fnsig(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type )
{
	Function* fn = sc_data_ptr(data)->self_fn.fn;

	if( abii->is_entry( v.symbol() ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		entry_values[su_stream_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufi" );
		entry_values[su_buffer_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;
		arg_it->setName( ".arg.stro" );
		entry_values[su_stream_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufo" );
		entry_values[su_buffer_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		// Create virutal arguments
		create_virtual_args(v, data);
	} else {
		parent_class::create_fnargs(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_virtual_args, function_type ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

END_NS_SASL_CODE_GENERATOR();