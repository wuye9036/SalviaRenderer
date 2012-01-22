#include <sasl/include/semantic/ssa_constructor.h>
#include <sasl/include/semantic/deps_graph.h>
#include <sasl/include/semantic/ssa_context.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/program.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

using sasl::syntax_tree::program;
using sasl::syntax_tree::declaration;

using boost::make_shared;
using boost::shared_ptr;

#define SASL_VISITOR_TYPE_NAME ssa_constructor

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<ssa_graph> ssa_constructor::construct_ssa( program const& root )
{
	shared_ptr<ssa_graph> ret = make_shared<ssa_graph>();
	ret->ctxt = make_shared<ssa_context>();

	ssa_constructor constr( ret.get(), ret->ctxt.get() );
	constr.visit_child( root.as_handle<program>() );
	return ret;
}

template <typename NodeT> void ssa_constructor::visit_child( shared_ptr<NodeT> const& child )
{
	child->accept( this, NULL );
}

ssa_constructor::ssa_constructor( ssa_graph* dg, ssa_context* ctxt ): dg(dg), ctxt(ctxt)
{
}

SASL_VISIT_DEF( program )
{
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child(decl);
	}
}
// expression
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
SASL_VISIT_DEF_UNIMPL( declarator );
SASL_VISIT_DEF_UNIMPL( variable_declaration );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( builtin_type );
SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( struct_type );
SASL_VISIT_DEF_UNIMPL( alias_type );
SASL_VISIT_DEF_UNIMPL( parameter );
SASL_VISIT_DEF_UNIMPL( function_type );

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

END_NS_SASL_SEMANTIC();

