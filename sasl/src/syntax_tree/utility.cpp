#include <sasl/include/syntax_tree/utility.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

#define SAFE_ACCEPT( node_handle ) if( node_handle ) { (node_handle)->accept(this); }
class follow_up_visitor : public syntax_tree_visitor{
public:
	follow_up_visitor( boost::function<void( node& )> applied ): applied( applied ){}

	// expression
	virtual void visit( unary_expression& v ) {
		SAFE_ACCEPT( v.expr );
		applied( v );
	}
	virtual void visit( cast_expression& v) {
		SAFE_ACCEPT( v.casted_type );
		SAFE_ACCEPT( v.expr );
		applied ( v );
	}
	virtual void visit( binary_expression& v ){
		SAFE_ACCEPT( v.left_expr );
		SAFE_ACCEPT( v.right_expr );
		applied( v );
	}
	virtual void visit( expression_list& v ) {
		visit( v.exprs );
		applied( v );
	}
	virtual void visit( cond_expression& v ) {
		SAFE_ACCEPT( v.cond_expr );
		SAFE_ACCEPT( v.yes_expr );
		SAFE_ACCEPT( v.no_expr );
		applied( v );
	}
	virtual void visit( index_expression& v ) {
		SAFE_ACCEPT( v.expr );
		SAFE_ACCEPT( v.index_expr );
		applied( v );
	}
	virtual void visit( call_expression& v ){
		SAFE_ACCEPT( v.expr );
		visit( v.args );
		applied( v );
	}
	virtual void visit( member_expression& v ) {
		SAFE_ACCEPT( v.expr );
		applied( v );
	}
	virtual void visit( constant_expression& v ) {
		applied( v );
	}
	virtual void visit( variable_expression& v ) {
		applied( v );
	}

	// declaration & type specifier
	virtual void visit( initializer& /*v*/ ){
		// do nothing
	}
	virtual void visit( expression_initializer& v ) {
		SAFE_ACCEPT( v.init_expr );
		applied( v );
	}
	virtual void visit( member_initializer& v ) {
		visit( v.sub_inits );
		applied( v );
	}
	virtual void visit( declaration& /*v*/ ) {
		// do nothing
	}
	virtual void visit( variable_declaration& v ){
		SAFE_ACCEPT( v.type_info );
		SAFE_ACCEPT( v.init );
		applied( v );
	}
	virtual void visit( type_definition& v ){
		SAFE_ACCEPT( v.type_info );
		applied( v );
	}
	virtual void visit( type_specifier& /*v*/ ){
		// do nothing
	}
	virtual void visit( buildin_type& v ){
		applied( v );
	}
	virtual void visit( array_type& v ){
		SAFE_ACCEPT( v.elem_type );
		visit( v.array_lens );
		applied( v );
	}
	virtual void visit( struct_type& v ){
		visit( v.decls );
		applied( v );
	}
	virtual void visit( parameter& v ){
		SAFE_ACCEPT( v.init );
		SAFE_ACCEPT( v.param_type );
		applied( v );
	}
	virtual void visit( function_type& v ) {
		SAFE_ACCEPT( v.retval_type );
		visit( v.params );
		SAFE_ACCEPT( v.body );
		applied( v );
	}

	// statement
	virtual void visit( statement& /*v*/ ){
		// do nothing
	}
	virtual void visit( declaration_statement& v ){
		SAFE_ACCEPT( v.decl );
		applied( v );
	}
	virtual void visit( if_statement& v ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.yes_stmt );
		SAFE_ACCEPT( v.no_stmt );
		applied( v );
	}
	virtual void visit( while_statement& v ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.body );
		applied( v );
	}
	virtual void visit( dowhile_statement& v ){
		SAFE_ACCEPT( v.body );
		SAFE_ACCEPT( v.cond );
		applied( v );
	}
	virtual void visit( for_statement& v ){
		SAFE_ACCEPT( v.init );
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.iter );
		SAFE_ACCEPT( v.body );
		applied( v );
	}
	virtual void visit( case_label& v ){
		SAFE_ACCEPT( v.expr );
		applied( v );
	}
	virtual void visit( ident_label& v ){
		applied( v );
	}
	virtual void visit( switch_statement& v ){
		SAFE_ACCEPT( v.cond );
		SAFE_ACCEPT( v.stmts );
		applied( v );
	}
	virtual void visit( compound_statement& v ){
		visit( v.stmts );
		applied( v );
	}
	virtual void visit( expression_statement& v ){
		SAFE_ACCEPT( v.expr );
		applied( v );
	}
	virtual void visit( jump_statement& v ){
		SAFE_ACCEPT( v.jump_expr );
		applied( v );
	}

	// program
	virtual void visit( program& v ){
		visit( v.decls );
		applied( v );
	}

private:
	template <typename ContainerT>
	void visit( ContainerT& v ){
		for( typename ContainerT::iterator it = v.begin(); it != v.end(); ++it ){
			SAFE_ACCEPT( *it );
		}
	}
	boost::function<void( node& )> applied;
};

void follow_up_traversal( boost::shared_ptr<node> root, boost::function<void( node& )> on_visit ){
	follow_up_visitor fuv( on_visit );
	if( root ){
		root->accept( &fuv );
	}
}

END_NS_SASL_SYNTAX_TREE();
