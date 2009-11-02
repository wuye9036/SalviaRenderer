#ifndef SASL_NODE_TYPE_QUALIFIER_H
#define SASL_NODE_TYPE_QUALIFIER_H

#include "ast_node.h"

class ast_node_type_qualifier: public ast_node{
public:
	static h_ast_node_type_qualifier create(const type_qualifiers& qual);

	//inherited
	virtual std::string get_typename() const;
	virtual void visit(ast_visitor* visitor);
	virtual void release();

	//others
	type_qualifiers get_qualifier() const;

protected:
	ast_node_type_qualifier(const type_qualifiers& type_qual)
		:ast_node( ast_node_types::type_qual ), qual_(type_qual)
	{}
private:
	type_qualifiers qual_;
};


#endif