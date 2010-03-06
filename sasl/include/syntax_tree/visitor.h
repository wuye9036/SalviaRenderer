#ifndef SASL_SYNTAX_TREE_VISITOR
#define SASL_SYNTAX_TREE_VISITOR

struct unary_expression;
struct cast_expression;
struct expression_list;
struct cond_expression;
struct index_expression;
struct call_expression;
struct member_expression;
struct identifier;
struct constant_expression;
struct binary_expression;
struct constant;

class syntax_tree_visitor{
public:
	virtual void visit( unary_expression& v ) = 0;
	virtual void visit( cast_expression& v) = 0;
	virtual void visit( binary_expression& v ) = 0;
	virtual void visit( expression_list& v ) = 0;
	virtual void visit( cond_expression& v ) = 0;
	virtual void visit( index_expression& v ) = 0;
	virtual void visit( call_expression& v ) = 0;
	virtual void visit( member_expression& v ) = 0;

	virtual void visit( constant_expression& v ) = 0;
	virtual void visit( constant& v ) = 0;
	virtual void visit( identifier& v ) = 0;
};

#endif