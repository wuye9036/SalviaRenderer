#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class type_converter;
	}
	namespace syntax_tree{
		struct type_specifier;
		struct node;	
	}
}

namespace llvm{
	class Constant;
	class Function;
	class Value;
}

BEGIN_NS_SASL_CODE_GENERATOR();

struct cgllvm_sctxt_data;

// Code generation for SISD( Single Instruction Single Data )
class cgllvm_sisd: public cgllvm_impl{
public:
	bool generate(
		sasl::semantic::module_si* mod,
		sasl::semantic::abi_info const* abii
	);

	SASL_VISIT_DCL( program );
	SASL_VISIT_DCL( function_type );
protected:
	// It is called in program visitor BEFORE declaration was visited.
	// If any additional initialization you want to add before visit, override it.
	// DONT FORGET call parent function before your code.
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	virtual bool create_mod( sasl::syntax_tree::program& v ) = 0;

	// Override node_ctxt of cgllvm_impl
	template <typename NodeT >
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<NodeT> const& v, bool create_if_need = false ){
		return cgllvm_impl::node_ctxt<NodeT, cgllvm_sctxt>(v, create_if_need);
	}
	cgllvm_sctxt* node_ctxt( sasl::syntax_tree::node&, bool create_if_need = false );

	// Get zero filled value of any type.
	llvm::Constant* zero_value( boost::shared_ptr<sasl::syntax_tree::type_specifier> );
	llvm::Constant* zero_value( llvm::Type const* );

	// LLVM code generator Utilities
	llvm::Value* load( boost::any* data );
	llvm::Value* load( cgllvm_sctxt* data );
	
	void store( llvm::Value*, boost::any* data );
	void store( llvm::Value*, cgllvm_sctxt* data );
	
	void create_alloca( cgllvm_sctxt* data, std::string const& name );

	void restart_block( boost::any* data );
	void clear_empty_blocks( llvm::Function* fn );

	// For type conversation.
	boost::function<cgllvm_sctxt*( boost::shared_ptr<sasl::syntax_tree::node> const& )> ctxt_getter;
	boost::shared_ptr< ::sasl::semantic::type_converter > typeconv;

	cgllvm_modimpl* mod_ptr();
};

cgllvm_sctxt const * sc_ptr( const boost::any& any_val  );
cgllvm_sctxt* sc_ptr( boost::any& any_val );

cgllvm_sctxt const * sc_ptr( const boost::any* any_val  );
cgllvm_sctxt* sc_ptr( boost::any* any_val );

cgllvm_sctxt_data* sc_inner_ptr( boost::any* any_val );
cgllvm_sctxt_data const* sc_inner_ptr( boost::any const* any_val );

END_NS_SASL_CODE_GENERATOR();

#endif