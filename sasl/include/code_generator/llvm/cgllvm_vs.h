#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H

#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>

#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/value_init.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class caster_t;
		class module_si;
	}
	namespace syntax_tree{
		struct expression;
		struct tynode;
		struct node;
	}
}

namespace llvm{
	class PointerType;
	class StructType;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_modvs;

class cgllvm_vs: public cgllvm_sisd
{
public:
	typedef cgllvm_sisd parent_class;

	cgllvm_vs();

	// expressions
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( index_expression );
	
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( alias_type );

private:
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	// Overrides them for generating entry function if need.
	SASL_SPECIFIC_VISIT_DCL( create_fnsig, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_virtual_args, function_type );

	SASL_SPECIFIC_VISIT_DCL( return_statement, jump_statement );

	SASL_SPECIFIC_VISIT_DCL( visit_global_declarator, declarator );

	bool is_entry( llvm::Function* ) const;

	virtual bool create_mod( sasl::syntax_tree::program& v );
	cgllvm_modvs* mod_ptr();

	boost::shared_ptr<sasl::semantic::symbol> find_symbol( cgllvm_sctxt* data, std::string const& str );

	value_t si_to_value( salviar::storage_info* si );

	void create_entry_params();
	void add_entry_param_type( salviar::storage_classifications st, std::vector< llvm::Type const* >& par_types );
	void fill_llvm_type_from_si( salviar::storage_classifications st );
	void copy_to_result( boost::shared_ptr<sasl::syntax_tree::expression> const& );
	void copy_to_agg_result( cgllvm_sctxt* data );

	llvm::Function* entry_fn;
	sasl::semantic::symbol* entry_sym;

	value_t param_values[salviar::storage_classifications_count];

	std::vector<builtin_types> entry_param_tys[salviar::storage_classifications_count];
	std::vector< llvm::Type const* > entry_params_types[salviar::storage_classifications_count];
	boost::value_initialized<llvm::StructType*> entry_params_structs[salviar::storage_classifications_count];
};

END_NS_SASL_CODE_GENERATOR();

#endif