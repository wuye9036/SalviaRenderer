#ifndef SASL_SEMANTIC_SEMANTIC_ANALYSER_IMPL_H
#define SASL_SEMANTIC_SEMANTIC_ANALYSER_IMPL_H

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
	}
	namespace syntax_tree{
		struct node;
		struct function_type;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;
class type_converter;
class global_si;
struct sacontext;

class semantic_analyser_impl: public ::sasl::syntax_tree::syntax_tree_visitor{
public:
	semantic_analyser_impl();

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
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( type_specifier );
	SASL_VISIT_DCL( buildin_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( struct_type );
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

	// program
	SASL_VISIT_DCL( program );

	boost::shared_ptr<global_si> global_semantic_info() const;

private:
	template <typename NodeT> sacontext& visit_child( sacontext& child_ctxt, boost::shared_ptr<NodeT> child );
	template <typename NodeT> sacontext& visit_child( sacontext& child_ctxt, const sacontext& init_data, boost::shared_ptr<NodeT> child );
	template <typename NodeT> sacontext& visit_child(
		sacontext& child_ctxt, const sacontext& init_data,
		boost::shared_ptr<NodeT> child, boost::shared_ptr<NodeT>& generated_node 
		);
	void register_type_converter( const sacontext& ctxt );
	void register_buildin_functions( const sacontext& child_ctxt_init );
	void register_buildin_types();

	void buildin_type_convert(
		boost::shared_ptr< ::sasl::syntax_tree::node >,
		boost::shared_ptr< ::sasl::syntax_tree::node >
		);

	boost::shared_ptr<global_si> gctxt;

	boost::shared_ptr<type_converter> typeconv;
	boost::shared_ptr<symbol> cursym;
	bool is_local;
};

END_NS_SASL_SEMANTIC();

#endif
