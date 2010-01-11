#ifndef SASL_SYNTAX_TREE_VISITOR
#define SASL_SYNTAX_TREE_VISITOR

struct constant_expression;
struct binary_expression;
struct constant;

class syntax_tree_visitor{
public:
	virtual void visit( constant_expression& v ) = 0;
	virtual void visit( binary_expression& v ) = 0;
	virtual void visit( constant& v ) = 0;
};

#endif