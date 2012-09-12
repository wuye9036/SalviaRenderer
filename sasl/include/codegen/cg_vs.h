#ifndef SASL_CODEGEN_CG_VS_H
#define SASL_CODEGEN_CG_VS_H

#include <sasl/include/codegen/cg_sisd.h>

#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/value_init.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class caster_t;
		class module_semantic;
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
	class TargetData;
}

BEGIN_NS_SASL_CODEGEN();

class cg_vs: public cg_sisd
{
public:
	typedef cg_sisd parent_class;

	cg_vs();
	~cg_vs();

	// expressions
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( alias_type );

private:
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	// Binary logical operators.
	SASL_SPECIFIC_VISIT_DCL( bin_logic, binary_expression );

	// Overrides them for generating entry function if need.
	SASL_SPECIFIC_VISIT_DCL( create_fnsig, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_virtual_args, function_type );

	SASL_SPECIFIC_VISIT_DCL( visit_return, jump_statement );

	bool is_entry( llvm::Function* ) const;

	cg_module_impl* mod_ptr();

	cg_value layout_to_value(salviar::sv_layout* si, bool copy_from_input);

	// If ctxt is NULL, the generated value and type will be cached.
	// Return true if context is fetched from cache.
	bool layout_to_node_context(
		node_context* ctxt, salviar::sv_layout* si,
		bool store_to_existed_value, bool copy_from_input);

	void create_entry_params();
	void add_entry_param_type( salviar::sv_usage st, std::vector< llvm::Type* >& par_types );
	void fill_llvm_type_from_si( salviar::sv_usage st );
	void copy_to_result( boost::shared_ptr<sasl::syntax_tree::expression> const& );
	void copy_to_agg_result( node_context* data );

	llvm::Function* entry_fn;
	sasl::semantic::symbol* entry_sym;

	cg_value param_values[salviar::storage_usage_count];

	std::vector<builtin_types> entry_param_tys[salviar::storage_usage_count];
	std::vector< llvm::Type* > entry_params_types[salviar::storage_usage_count];
	boost::value_initialized<llvm::StructType*> entry_params_structs[salviar::storage_usage_count];

	typedef boost::unordered_map<salviar::semantic_value, node_context*> input_copies_dict;
	input_copies_dict input_copies_;
};

END_NS_SASL_CODEGEN();

#endif