#ifndef SASL_SEMANTIC_SEMANTIC_ANALYSER_H
#define SASL_SEMANTIC_SEMANTIC_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/syntax_tree/visitor.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl{
	namespace common{
		class compiler_info_manager;
		struct token_t;
	}
	namespace syntax_tree{
		struct node;
	}
}

namespace softart{
	enum languages;
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;
class caster_t;
class module_si;
class storage_si;
struct sacontext;

class semantic_analyser: public ::sasl::syntax_tree::syntax_tree_visitor{
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
	SASL_VISIT_DCL( parameter );
	SASL_VISIT_DCL( function_type );

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

	boost::shared_ptr<module_si> const& module_semantic_info() const;
private:
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, boost::shared_ptr<NodeT> child );
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, const boost::any& init_data, boost::shared_ptr<NodeT> child );
	template <typename NodeT> boost::any& visit_child(
		boost::any& child_ctxt, const boost::any& init_data,
		boost::shared_ptr<NodeT> child, boost::shared_ptr<NodeT>& generated_node 
		);

	void parse_semantic(
		boost::shared_ptr<sasl::common::token_t> const& sem_tok,
		boost::shared_ptr<sasl::common::token_t> const& sem_idx_tok,
		boost::shared_ptr<storage_si> const& ssi
		);

	void add_cast( const boost::any& ctxt );
	void register_builtin_functions( const boost::any& child_ctxt_init );

	class function_register{
	public:
		typedef boost::shared_ptr<sasl::syntax_tree::tynode> type_handle_t;

		function_register(
			semantic_analyser& owner,
			boost::any const& ctxt_init,
			boost::shared_ptr<sasl::syntax_tree::function_type> const& fn,
			bool is_intrinsic
			);
		function_register( function_register const& );

		function_register& operator % ( type_handle_t const& par_type );
		void operator >> ( type_handle_t const& ret_type );

		function_register& p( type_handle_t const& par_type );
		void r( type_handle_t const& ret_type );

	private:
		function_register& operator = ( function_register const& );

		boost::any const& ctxt_init;
		boost::shared_ptr<sasl::syntax_tree::function_type> fn;
		semantic_analyser& owner;
		bool is_intrinsic;
	};

	function_register register_function( boost::any const& child_ctxt_init, std::string const& name );
	function_register register_intrinsic( boost::any const& child_ctxt_init, std::string const& name );

	void register_builtin_types();

	void builtin_tecov(
		boost::shared_ptr< ::sasl::syntax_tree::node >,
		boost::shared_ptr< ::sasl::syntax_tree::node >
		);

	boost::shared_ptr<module_si> msi;
	boost::shared_ptr<caster_t> caster;
};

END_NS_SASL_SEMANTIC();

#endif
