#include <sasl/include/syntax_tree/utility.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/enable_if.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/preprocessor.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using std::vector;

using boost::any;
using boost::any_cast;
using boost::is_base_of;
using boost::shared_polymorphic_cast;
using boost::shared_ptr;

BEGIN_NS_SASL_SYNTAX_TREE();

#define SAFE_ACCEPT( node_handle ) if( node_handle ) { (node_handle)->accept(this, data); }
class follow_up_visitor : public syntax_tree_visitor{
public:
	follow_up_visitor( boost::function<void( node&, ::boost::any* )> applied ): applied( applied ){}

	// expression
	SASL_VISIT_DCL( unary_expression ) {
		SAFE_ACCEPT( v.expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( cast_expression ) {
		SAFE_ACCEPT( v.casted_type );
		SAFE_ACCEPT( v.expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( binary_expression ){
		SAFE_ACCEPT( v.left_expr );
		SAFE_ACCEPT( v.right_expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( expression_list ) {
		visit( v.exprs, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( cond_expression ) {
		SAFE_ACCEPT( v.cond_expr );
		SAFE_ACCEPT( v.yes_expr );
		SAFE_ACCEPT( v.no_expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( index_expression ) {
		SAFE_ACCEPT( v.expr );
		SAFE_ACCEPT( v.index_expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( call_expression ){
		SAFE_ACCEPT( v.expr );
		visit( v.args, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( member_expression ) {
		SAFE_ACCEPT( v.expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( constant_expression ) {
		applied( v, data );
	}
	SASL_VISIT_DCL( variable_expression ) {
		applied( v, data );
	}

	// declaration & type specifier
	SASL_VISIT_INLINE_DEF_UNIMPL( initializer );

	SASL_VISIT_DCL( expression_initializer ) {
		SAFE_ACCEPT( v.init_expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( member_initializer ) {
		visit( v.sub_inits, data );
		applied( v, data );
	}
	SASL_VISIT_INLINE_DEF_UNIMPL( declaration );
	SASL_VISIT_DCL( variable_declaration ){
		SAFE_ACCEPT( v.type_info );
		visit( v.declarators, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( declarator ){
		SAFE_ACCEPT( v.init );
		applied( v, data );
	}
	SASL_VISIT_DCL( type_definition ){
		SAFE_ACCEPT( v.type_info );
		applied( v, data );
	}
	SASL_VISIT_INLINE_DEF_UNIMPL( type_specifier );
	SASL_VISIT_DCL( builtin_type ){
		applied( v, data );
	}
	SASL_VISIT_DCL( array_type ){
		SAFE_ACCEPT( v.elem_type );
		visit( v.array_lens, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( struct_type ){
		visit( v.decls, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( alias_type ){
		visit( v, data );
	}
	SASL_VISIT_DCL( parameter ){
		SAFE_ACCEPT( v.init );
		SAFE_ACCEPT( v.param_type );
		applied( v, data );
	}
	SASL_VISIT_DCL( function_type ) {
		SAFE_ACCEPT( v.retval_type );
		visit( v.params, data );
		SAFE_ACCEPT( v.body );
		applied( v, data );
	}

	// statement
	SASL_VISIT_INLINE_DEF_UNIMPL( statement );

	SASL_VISIT_DCL( declaration_statement ){
		SAFE_ACCEPT( v.decl );
		applied( v, data );
	}
	SASL_VISIT_DCL( if_statement ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.yes_stmt );
		SAFE_ACCEPT( v.no_stmt );
		applied( v, data );
	}
	SASL_VISIT_DCL( while_statement ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.body );
		applied( v, data );
	}
	SASL_VISIT_DCL( dowhile_statement ){
		SAFE_ACCEPT( v.body );
		SAFE_ACCEPT( v.cond );
		applied( v, data );
	}
	SASL_VISIT_DCL( for_statement ){
		SAFE_ACCEPT( v.init );
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.iter );
		SAFE_ACCEPT( v.body );
		applied( v, data );
	}
	SASL_VISIT_DCL( case_label ){
		SAFE_ACCEPT( v.expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( ident_label ){
		applied( v, data );
	}
	SASL_VISIT_DCL( switch_statement ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.stmts );
		applied( v, data );
	}
	SASL_VISIT_DCL( compound_statement ){
		visit( v.stmts, data );
		applied( v, data );
	}
	SASL_VISIT_DCL( expression_statement ){
		SAFE_ACCEPT( v.expr );
		applied( v, data );
	}
	SASL_VISIT_DCL( jump_statement ){
		SAFE_ACCEPT( v.jump_expr );
		applied( v, data );
	}

	// program
	SASL_VISIT_DCL( program ){
		visit( v.decls, data );
		applied( v, data );
	}

private:
	template <typename ContainerT>
	void visit( ContainerT& v, ::boost::any* data ){
		for( typename ContainerT::iterator it = v.begin(); it != v.end(); ++it ){
			SAFE_ACCEPT( *it );
		}
	}
	boost::function<void( node&, ::boost::any* )> applied;
};

void follow_up_traversal( boost::shared_ptr<node> root, boost::function<void( node&, ::boost::any* )> on_visit ){
	follow_up_visitor fuv( on_visit );
	if( root ){
		root->accept( &fuv, NULL );
	}
}

boost::shared_ptr<builtin_type> create_builtin_type( const builtin_types& btc )
{
	boost::shared_ptr<builtin_type> ret = create_node<builtin_type>( token_t::null() );
	ret->value_typecode = btc;
	return ret;
}

template< typename NodeT >
void store_node_to_data( any* lhs, shared_ptr< NodeT > rhs ){
	if( rhs ){
		*lhs = shared_polymorphic_cast<node>( rhs );
	} else {
		*lhs = shared_ptr<node>();
	}
}

#define COPY_VALUE_ITEM( r, dest_src, member ) \
	BOOST_PP_TUPLE_ELEM(2, 0, dest_src)->member = BOOST_PP_TUPLE_ELEM(2, 1, dest_src).member;

#define SASL_SWALLOW_CLONE_NODE( output, v, node_type, member_seq ) \
	::boost::shared_ptr< node_type > cloned	= create_node< node_type >( token_t::null() ); \
	BOOST_PP_SEQ_FOR_EACH( COPY_VALUE_ITEM, (cloned, v), member_seq ); \
	store_node_to_data( (output), cloned );

template<typename T> void copy_from_any( T& lhs, const boost::any& rhs ){
	lhs = any_cast<T>(rhs);
}

template<typename NodeT> void copy_from_any( shared_ptr<NodeT>& lhs, const any& rhs, EFLIB_ENABLE_IF_PRED2(is_base_of, node, NodeT) ){
	shared_ptr<node> any_v = any_cast< shared_ptr<node> >( rhs );
	if ( any_v ){
		lhs = shared_polymorphic_cast<NodeT>(any_v);
	} else {
		lhs.reset();
	}
}

#define DEEPCOPY_VALUE_ITEM( r, dest_src, member )	\
	visit( BOOST_PP_TUPLE_ELEM(2, 1, dest_src).member, &member_dup );	\
	copy_from_any( BOOST_PP_TUPLE_ELEM(2, 0, dest_src)->member, member_dup );

#define SASL_DEEP_CLONE_NODE( dest_any_ptr, src_v_ref, node_type, member_seq )	\
	::boost::shared_ptr< node_type > cloned	= create_node< node_type >( token_t::null() ); \
	boost::any member_dup; \
	BOOST_PP_SEQ_FOR_EACH( DEEPCOPY_VALUE_ITEM, (cloned, src_v_ref), member_seq ); \
	store_node_to_data( (dest_any_ptr), cloned );

#define SASL_CLONE_NODE_FUNCTION_DEF( clone_mode, node_type, member_seq )	\
	SASL_VISIT_DCL( node_type ){	\
		EFLIB_ASSERT( data, "Data parameter must not be NULL, it is used to feedback cloned node." );	\
		SASL_##clone_mode##_CLONE_NODE( data, v, node_type, member_seq );	\
	}

class swallow_duplicator: public syntax_tree_visitor{
public:
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, unary_expression, (op)(expr) );

	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, cast_expression, (casted_type)(expr) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, binary_expression, (op)(left_expr)(right_expr) );
	SASL_VISIT_INLINE_DEF_UNIMPL( expression_list );
	SASL_VISIT_INLINE_DEF_UNIMPL( cond_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( index_expression );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, call_expression, (expr)(args) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, member_expression, (expr)(member) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, constant_expression, (value_tok)(ctype) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, variable_expression, (var_name) );

	// declaration & type specifier
	SASL_VISIT_INLINE_DEF_UNIMPL( initializer );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, expression_initializer, (init_expr) );
	SASL_VISIT_INLINE_DEF_UNIMPL( member_initializer );
	SASL_VISIT_INLINE_DEF_UNIMPL( declaration );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, variable_declaration, (type_info)(declarators) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, declarator, (name)(init)(semantic)(semantic_index) );
	SASL_VISIT_INLINE_DEF_UNIMPL( type_definition );
	SASL_VISIT_INLINE_DEF_UNIMPL( type_specifier );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, builtin_type, (value_typecode)(qual)(tok) );
	SASL_VISIT_INLINE_DEF_UNIMPL( array_type );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, struct_type, (name)(decls) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, alias_type, (alias) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, parameter, (param_type)(name)(init) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, function_type, (name)(retval_type)(params)(body)(value_typecode)(qual) );

	// statement
	SASL_VISIT_INLINE_DEF_UNIMPL( statement );

	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, declaration_statement, (decl) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, if_statement, (cond)(yes_stmt)(no_stmt) );
	SASL_VISIT_INLINE_DEF_UNIMPL( while_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( dowhile_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( for_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( case_label );
	SASL_VISIT_INLINE_DEF_UNIMPL( ident_label );
	SASL_VISIT_INLINE_DEF_UNIMPL( switch_statement );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, compound_statement, (stmts) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, expression_statement, (expr) );
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, jump_statement, (code)(jump_expr) );

	// program
	SASL_CLONE_NODE_FUNCTION_DEF( SWALLOW, program, (decls)(name) );
};

class deep_duplicator: public syntax_tree_visitor{
public:
	SASL_CLONE_NODE_FUNCTION_DEF( DEEP, unary_expression, (op)(expr)(tok) );

	SASL_VISIT_INLINE_DEF_UNIMPL( cast_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( binary_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( expression_list );
	SASL_VISIT_INLINE_DEF_UNIMPL( cond_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( index_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( call_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( member_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( constant_expression );
	SASL_VISIT_INLINE_DEF_UNIMPL( variable_expression );

	// declaration & type specifier
	SASL_VISIT_INLINE_DEF_UNIMPL( initializer );
	SASL_VISIT_INLINE_DEF_UNIMPL( expression_initializer );
	SASL_VISIT_INLINE_DEF_UNIMPL( member_initializer );
	SASL_VISIT_INLINE_DEF_UNIMPL( declaration );
	SASL_VISIT_INLINE_DEF_UNIMPL( variable_declaration );
	SASL_VISIT_INLINE_DEF_UNIMPL( declarator );
	SASL_VISIT_INLINE_DEF_UNIMPL( type_definition );
	SASL_VISIT_INLINE_DEF_UNIMPL( type_specifier );

	SASL_CLONE_NODE_FUNCTION_DEF( DEEP, builtin_type, (value_typecode)(qual)(tok) );

	SASL_VISIT_INLINE_DEF_UNIMPL( array_type );
	SASL_VISIT_INLINE_DEF_UNIMPL( struct_type );
	SASL_VISIT_INLINE_DEF_UNIMPL( alias_type );
	SASL_VISIT_INLINE_DEF_UNIMPL( parameter );
	SASL_VISIT_INLINE_DEF_UNIMPL( function_type );

	// statement
	SASL_VISIT_INLINE_DEF_UNIMPL( statement );

	SASL_VISIT_INLINE_DEF_UNIMPL( declaration_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( if_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( while_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( dowhile_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( for_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( case_label );
	SASL_VISIT_INLINE_DEF_UNIMPL( ident_label );
	SASL_VISIT_INLINE_DEF_UNIMPL( switch_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( compound_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( expression_statement );
	SASL_VISIT_INLINE_DEF_UNIMPL( jump_statement );

	// program
	SASL_VISIT_INLINE_DEF_UNIMPL( program );

	// If value is "value semantic", copy it as raw data.
	template <typename ValueT> void visit( ValueT& v, boost::any* data ){
		*data = v;
	}

	// If value is "value semantic", copy it as raw data.
	template <typename NodeT> void visit( vector< shared_ptr<NodeT> > & v, boost::any* data ){
		vector< shared_ptr<NodeT> > out_v(v.size());
		BOOST_FOREACH( shared_ptr<NodeT> item, v ){
			boost::any cloned;
			visit( item, &cloned );
			out_v.push_back( shared_polymorphic_cast<NodeT>( boost::any_cast< shared_ptr<node> > (cloned) ) );
		}
		*data = out_v;
	}
};

template <typename ValueT>
ValueT process_node( ::boost::shared_ptr<node> src, syntax_tree_visitor* v ){
	EFLIB_ASSERT_AND_IF( src && v, "The input parameter is unavaliable!" ){
		return src;
	}

	::boost::any result_val;
	src->accept( v, &result_val );
	return ::boost::any_cast< ValueT >(result_val);
}

boost::shared_ptr<node> duplicate( ::boost::shared_ptr<node> src ){
	swallow_duplicator dup;
	return process_node< ::boost::shared_ptr<node> >( src, &dup );
}

boost::shared_ptr<node> deep_duplicate( ::boost::shared_ptr<node> src ){
	deep_duplicator dup;
	return process_node< ::boost::shared_ptr<node> >( src, &dup );
}

END_NS_SASL_SYNTAX_TREE();
