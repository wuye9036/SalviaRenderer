#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

#include <vector>

namespace sasl{
	namespace semantic{
		class type_converter;
	}
	namespace syntax_tree{
		struct tynode;
		struct node;	
	}
}

namespace llvm{
	class BasicBlock;
	class Constant;
	class ConstantInt;
	class ConstantFP;
	class Function;
	class Value;
}

BEGIN_NS_SASL_CODE_GENERATOR();

struct cgllvm_sctxt_data;
struct cgllvm_sctxt_env;

template<typename BuilderT> class llext;

template<typename BuilderT> class llvalue;
template<typename BuilderT, unsigned int Bits, bool IsSigned> class llv_int;
template<typename BuilderT> class llv_fp;
template<typename BuilderT> class llaggregated;

template<typename ElementT> class llvar;
template<typename ElementT> class llvector;
template<typename ElementT> class llarray;

// Code generation for SISD( Single Instruction Single Data )
class cgllvm_sisd: public cgllvm_impl{
public:
	~cgllvm_sisd();
	bool generate(
		sasl::semantic::module_si* mod,
		sasl::semantic::abi_info const* abii
	);

	SASL_VISIT_DCL( program );

	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( constant_expression );
	
	SASL_VISIT_DCL( call_expression );

	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( function_type );
	SASL_VISIT_DCL( struct_type );

	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( parameter );

	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( jump_statement );
	SASL_VISIT_DCL( expression_statement );

protected:
	typedef llvalue<llvm::DefaultIRBuilder> llval;
	typedef llv_fp<llvm::DefaultIRBuilder> llfloat;
	typedef llv_int<llvm::DefaultIRBuilder, 32, true > lli32;
	typedef llaggregated<llvm::DefaultIRBuilder> llagg;

	typedef llvector<llfloat>	fvector;
	typedef llvector<lli32>		i32vector;

	SASL_SPECIFIC_VISIT_DCL( process_intrinsics, program );

	// It is called in program visitor BEFORE declaration was visited.
	// If any additional initialization you want to add before visit, override it.
	// DONT FORGET call parent function before your code.
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	// Called by function_type visitor.
	SASL_SPECIFIC_VISIT_DCL( create_fnsig, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnbody, function_type );

	SASL_SPECIFIC_VISIT_DCL( return_statement, jump_statement );

	SASL_SPECIFIC_VISIT_DCL( visit_member_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_global_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_local_declarator, declarator );

	SASL_SPECIFIC_VISIT_DCL( bin_assign, binary_expression );
	// SASL_SPECIFIC_VISIT_DCL( bin_arith, binary_expression );

	virtual bool create_mod( sasl::syntax_tree::program& v ) = 0;

	// Override node_ctxt of cgllvm_impl
	template <typename NodeT >
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<NodeT> const& v, bool create_if_need = false ){
		return cgllvm_impl::node_ctxt<NodeT, cgllvm_sctxt>(v, create_if_need);
	}
	cgllvm_sctxt* node_ctxt( sasl::syntax_tree::node&, bool create_if_need = false );

	// LLVM code generator Utilities
	llvm::Constant* zero_value( boost::shared_ptr<sasl::syntax_tree::tynode> typespec );

	//llvm::Value* load( boost::any* data );
	//llvm::Value* load( cgllvm_sctxt* data );
	//
	//llvm::Value* load_ptr( cgllvm_sctxt* data );
	//void store( llvm::Value*, boost::any* data );
	//void store( llvm::Value*, cgllvm_sctxt* data );

	llvm::Value* to_abi( builtin_types hint, llvm::Value* v );
	llvm::Value* from_abi( builtin_types hint, llvm::Value* v );

	void mask_to_indexes( char index[4], uint32_t mask );

	template <typename ElementT>
	llvector<ElementT> mul_vm(
		llvm::Value* v, llvm::Value* m,
		size_t vec_size, size_t mat_vec_size,
		llvm::Type const* ret_type
		);

	template <typename ElementT>
	llvector<ElementT> mul_mv(
		llvm::Value* m, llvm::Value* v,
		size_t vec_size, size_t n_vec,
		llvm::Type const* ret_type
		);
	template <typename ElementT>
	ElementT dot_prod(
		llvm::Value* lhs, llvm::Value* rhs,
		size_t vec_size,
		llvm::Type const* ret_type
		);

	void create_alloca( cgllvm_sctxt* data, std::string const& name );

	void restart_block( boost::any* data, std::string const& name );
	void clear_empty_blocks( llvm::Function* fn );

	// For type conversation.
	boost::function<cgllvm_sctxt*( boost::shared_ptr<sasl::syntax_tree::node> const& )> ctxt_getter;
	boost::shared_ptr< ::sasl::semantic::type_converter > typeconv;

	cgllvm_modimpl* mod_ptr();

	boost::shared_ptr< llext<llvm::DefaultIRBuilder> > ext;
};

cgllvm_sctxt const * sc_ptr( const boost::any& any_val  );
cgllvm_sctxt* sc_ptr( boost::any& any_val );

cgllvm_sctxt const * sc_ptr( const boost::any* any_val  );
cgllvm_sctxt* sc_ptr( boost::any* any_val );

cgllvm_sctxt_data* sc_data_ptr( boost::any* any_val );
cgllvm_sctxt_data const* sc_data_ptr( boost::any const* any_val );

cgllvm_sctxt_env* sc_env_ptr( boost::any* any_val );
cgllvm_sctxt_env const* sc_env_ptr( boost::any const* any_val );

END_NS_SASL_CODE_GENERATOR();

#endif