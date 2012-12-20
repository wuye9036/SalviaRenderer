#ifndef SASL_SEMANTIC_SSA_CONSTRUCTOR_H
#define SASL_SEMANTIC_SSA_CONSTRUCTOR_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/syntax_tree/visitor.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;

class ssa_context;
class ssa_graph;
struct cg_function;
struct block_t;

enum scopes
{
	scp_global,
	scp_local,
	scp_member
};

struct ssa_data;

class ssa_constructor: public sasl::syntax_tree::syntax_tree_visitor
{
public:
	static boost::shared_ptr<ssa_graph> construct_ssa( sasl::syntax_tree::program const& root );

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
	SASL_VISIT_DCL( parameter_full );
	SASL_VISIT_DCL( function_full_def );

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

private:
	ssa_constructor( ssa_graph*, ssa_context* ctxt );
	ssa_constructor( ssa_constructor const& );
	ssa_constructor& operator = ( ssa_constructor const& );

	template <typename NodeT> void visit_child( boost::shared_ptr<NodeT> const& child );
	
	void connect( block_t* from, block_t* to );

	// States
	scopes			current_scope;
	symbol*			current_symbol;
	cg_function*	current_fn;
	block_t*		current_block;

	ssa_graph*		dg;
	ssa_context*	ctxt;
};

END_NS_SASL_SEMANTIC();

#endif
