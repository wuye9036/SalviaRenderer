#ifndef SASL_SEMANTIC_SEMANTIC_ANALYSER_H
#define SASL_SEMANTIC_SEMANTIC_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/default_hasher.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

#include <vector>

namespace sasl{
	namespace common{
		EFLIB_DECLARE_CLASS_SHARED_PTR(diag_chat);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(token_t);
	}
	namespace syntax_tree{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(builtin_type);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(function_full_def);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(function_def);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(parameter);
		EFLIB_DECLARE_STRUCT_SHARED_PTR(expression);
	}
}

BEGIN_NS_SASL_SEMANTIC();

EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
EFLIB_DECLARE_CLASS_SHARED_PTR(caster_t);
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(node_semantic);
EFLIB_DECLARE_STRUCT_SHARED_PTR(sacontext);
EFLIB_DECLARE_STRUCT_SHARED_PTR(proto_info);

class semantic_analyser: public sasl::syntax_tree::syntax_tree_visitor{
public:
	semantic_analyser();

	// expression
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( index_expression );
	SASL_VISIT_DCL( call_expression );
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( constant_expression );
	SASL_VISIT_DCL( variable_expression );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( expression_initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( struct_type );
	SASL_VISIT_DCL( alias_type );
	SASL_VISIT_DCL( function_type );
	SASL_VISIT_DCL( parameter_full );
	SASL_VISIT_DCL( function_full_def );
	SASL_VISIT_DCL( parameter );
	SASL_VISIT_DCL( function_def );

	// statement
	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( if_statement );
	SASL_VISIT_DCL( while_statement );
	SASL_VISIT_DCL( dowhile_statement );
	SASL_VISIT_DCL( for_statement );
	SASL_VISIT_DCL( case_label );
	SASL_VISIT_DCL( ident_label );
	SASL_VISIT_DCL( switch_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( expression_statement );
	SASL_VISIT_DCL( jump_statement );
	SASL_VISIT_DCL( labeled_statement );

	// program
	SASL_VISIT_DCL( program );

	module_semantic_ptr const&	get_module_semantic() const;
	sasl::common::diag_chat*	get_diags() const;
	uint32_t					language() const;
	void						language( uint32_t );

private:
	template <typename NodeT> 
	boost::shared_ptr<NodeT> visit_child( boost::shared_ptr<NodeT> const& child, node_semantic** return_sem = NULL );
	template <typename ReturnNodeT, typename NodeT> 
	boost::shared_ptr<ReturnNodeT> visit_child( boost::shared_ptr<NodeT> const& child, node_semantic** return_sem = NULL );

	void parse_semantic(
		sasl::common::token_t_ptr const& sem_tok,
		sasl::common::token_t_ptr const& sem_idx_tok,
		node_semantic* ssi
		);
	
	node_semantic* get_node_semantic( sasl::syntax_tree::node* );
	node_semantic* get_node_semantic( sasl::syntax_tree::node_ptr const& );
	
	node_semantic* create_node_semantic( sasl::syntax_tree::node* );
	node_semantic* create_node_semantic( sasl::syntax_tree::node_ptr const& );

	node_semantic* get_or_create_semantic( sasl::syntax_tree::node* );
	node_semantic* get_or_create_semantic( sasl::syntax_tree::node_ptr const& );

	symbol* get_symbol( sasl::syntax_tree::node* );
	symbol* get_symbol( sasl::syntax_tree::node_ptr const& );

	static std::string unique_structure_name();
	void mark_intrin_invoked_recursive(symbol* sym);

	void mark_modified(sasl::syntax_tree::expression* expr);

	void initialize_casts();
	void initialize_operator_parameter_lrvs();
	void register_builtin_types();
	void register_builtin_functions2();
	void hold_generated_node( sasl::syntax_tree::node_ptr const& );
	void initialize_operator_parameter_to_lrv_lookup_table();

	void register_function2(
		eflib::fixed_string		const& name,
		std::vector<size_t>		const& proto_indexes,
		std::vector<proto_info> const& protos,
		bool is_intrinsic = false,
		bool is_partial_exec = false,
		bool is_constructor = false
		);
	void register_intrinsic2(
		eflib::fixed_string		const& name,
		std::vector<size_t>		const& proto_indexes,
		std::vector<proto_info> const& protos,
		bool partial_exec = false
		);
	void register_constructor2(
		eflib::fixed_string		const& name,
		std::vector<size_t>		const& proto_indexes,
		std::vector<proto_info> const& protos
		);

	void empty_caster(sasl::syntax_tree::node*, sasl::syntax_tree::node*);

	module_semantic_ptr			module_semantic_;
	caster_t_ptr				caster;
	sasl::common::diag_chat_ptr	diags;
	uint32_t					lang;
	sasl::syntax_tree::program*	prog_;

	// Global States
	typedef std::vector< boost::weak_ptr<
		sasl::syntax_tree::labeled_statement> >	label_list_t;
	typedef sasl::syntax_tree::function_def_ptr function_def_ptr;

	function_def_ptr			current_function;
	label_list_t				*label_list;
	sasl::syntax_tree::node_ptr	variable_to_initialized;
	sasl::syntax_tree::node_ptr generated_node;
	node_semantic*				generated_sem;

	struct parameter_lrvs
	{
		parameter_lrvs(
			lvalue_or_rvalue::id ret_lrv = lvalue_or_rvalue::unknown,
			lvalue_or_rvalue::id lrv_p0 = lvalue_or_rvalue::unknown,
			lvalue_or_rvalue::id lrv_p1 = lvalue_or_rvalue::unknown,
			lvalue_or_rvalue::id lrv_p2 = lvalue_or_rvalue::unknown
			);
		lvalue_or_rvalue::id ret_lrv;
		lvalue_or_rvalue::id param_lrvs[3];
	};

	boost::unordered_map<operators, parameter_lrvs>
			operator_parameter_lrvs_;
	bool	is_global_scope;
	symbol*	current_symbol;
	int		declaration_tid;
	int		member_counter;
};

END_NS_SASL_SEMANTIC();

#endif
