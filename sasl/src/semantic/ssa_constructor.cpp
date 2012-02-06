#include <sasl/include/semantic/ssa_constructor.h>
#include <sasl/include/semantic/ssa_context.h>
#include <sasl/include/semantic/ssa_graph.h>
#include <sasl/include/semantic/ssa_nodes.h>

#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/expression.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

using sasl::syntax_tree::program;
using sasl::syntax_tree::declaration;
using sasl::syntax_tree::declarator;
using sasl::syntax_tree::parameter;
using sasl::syntax_tree::statement;

using boost::make_shared;
using boost::shared_ptr;

using std::make_pair;

#define SASL_VISITOR_TYPE_NAME ssa_constructor

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<ssa_graph> ssa_constructor::construct_ssa( program const& root )
{
	shared_ptr<ssa_graph> ret = make_shared<ssa_graph>();
	ssa_constructor constr( ret.get(), ret->context() );
	constr.visit_child( root.as_handle<program>() );
	return ret;
}

template <typename NodeT> void ssa_constructor::visit_child( shared_ptr<NodeT> const& child )
{
	child->accept( this, NULL );
}

ssa_constructor::ssa_constructor( ssa_graph* dg, ssa_context* ctxt )
	: dg(dg), ctxt(ctxt)
	, current_fn(NULL)
{
}

void ssa_constructor::connect( block_t* from, block_t* to )
{
	from->succs.push_back( make_pair( (value_t*)NULL, to ) );
	to->preds.push_back( from );
}

SASL_VISIT_DEF( program )
{
	current_scope = scp_global;
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
SASL_VISIT_DEF( variable_expression )
{
	storage_si* ssi = v.dyn_siptr<storage_si>();
	if( ssi )
	{
		// Variable
		ctxt->attr(&v).var = ctxt->attr( ssi->declarator()->node().get() ).var;
	} 
	else
	{
		// Function name
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( expression_initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );

SASL_VISIT_DEF( declarator ){
	if( current_scope == scp_member ){
		return;
	}
	variable_t* var = ctxt->create_variable( current_fn, &v );
	ctxt->attr(&v).var = var;
}

SASL_VISIT_DEF( variable_declaration )
{
	BOOST_FOREACH( shared_ptr<declarator> const& decl, v.declarators )
	{
		visit_child( decl );
	}
}

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( builtin_type );
SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( struct_type );
SASL_VISIT_DEF_UNIMPL( alias_type );

SASL_VISIT_DEF( parameter )
{
	value_t* val = ctxt->create_value( NULL, &v );
	variable_t* var = ctxt->create_variable( current_fn, &v );
	ctxt->attr(&v).val = val;
	ctxt->attr(&v).var = var;
}

SASL_VISIT_DEF( function_type )
{
	function_t* fn = ctxt->create_function();
	ctxt->attr( &v ).fn = fn;

	current_fn = fn;

	if( v.body ){
		fn->entry	= ctxt->create_block( fn );
		fn->exit	= ctxt->create_block( fn );
		fn->retval	= ctxt->emit( fn->exit, instruction_t::phi );
		connect( fn->entry, fn->exit );
		current_block = fn->entry;
		BOOST_FOREACH( shared_ptr<parameter> const& param, v.params )
		{
			visit_child(param);
		}
		visit_child( v.body );
	}
}

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

SASL_VISIT_DEF( compound_statement )
{
	BOOST_FOREACH( shared_ptr<statement> const& stmt, v.stmts )
	{
		visit_child( stmt );
	}
}

SASL_VISIT_DEF_UNIMPL( expression_statement );
SASL_VISIT_DEF( jump_statement )
{
	if( v.code == jump_mode::_return )
	{
		if( v.jump_expr ){
			visit_child( v.jump_expr );
			value_t* expr_value = ctxt->load( v.jump_expr.get() );
			( (instruction_t*)current_fn->retval )->params.push_back( expr_value );
		}
		connect( current_block, current_fn->exit );
	} 
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}
SASL_VISIT_DEF_UNIMPL( labeled_statement );

END_NS_SASL_SEMANTIC();

