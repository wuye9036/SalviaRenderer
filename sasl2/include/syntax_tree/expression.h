#ifndef SASL_SYNTAX_TREE_EXPRESSION_H
#define SASL_SYNTAX_TREE_EXPRESSION_H

#include "../syntax_tree/node.h"
#include "../syntax_tree/constant.h"
#include "../syntax_tree/operator_literal.h"
#include "../syntax_tree/identifier.h"
#include "../../enums/operators.h"
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/fusion/adapted.hpp>

struct constant;
struct operator_literal;

struct expression: public node_impl<expression>{
	expression( syntax_node_types nodetype)
		:node_impl( nodetype, token_attr::handle_t() ){}
};

struct constant_expression: public expression{
	constant::handle_t value;
	void update(){
		value->update();
	}
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}
	constant_expression(): expression( syntax_node_types::node ){}
};

struct unary_expression: public expression{
	expression::handle_t expr;
	operator_literal::handle_t op;

	void update(){
		expr->update();
		op->update();
	}

	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}

	unary_expression(): expression( syntax_node_types::node ){}
};

struct cast_expression: public expression{
	identifier::handle_t type_ident;
	expression::handle_t expr;

	void update(){
		type_ident->update();
		expr->update();
	}

	void accept( syntax_tree_visitor* visitor){
		visitor->visit( *this );
	}
	
	cast_expression(): expression( syntax_node_types::node ){}
};

struct binary_expression: public expression {
	operator_literal::handle_t op;
	expression::handle_t left_expr;
	expression::handle_t right_expr;

	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}

	void update(){
		op->update();
		left_expr->update();
		right_expr->update();
	}

	binary_expression();
};

struct expression_list: public expression{
	std::vector< expression::handle_t > exprs;

	void accept( syntax_tree_visitor* visitor){
		visitor->visit(*this);
	}

	void update(){
		for(std::vector<expression::handle_t>::iterator it = exprs.begin(); it != exprs.end(); ++it){
			(*it)->update();
		}
	}

	expression_list(): expression( syntax_node_types::node ){}
};

struct cond_expression: public expression{
	expression::handle_t cond_expr;
	expression::handle_t yes_expr;
	expression::handle_t no_expr;
	
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit(*this);
	}

	void update(){
		cond_expr->update();
		yes_expr->update();
		no_expr->update();
	}

	cond_expression(): expression( syntax_node_types::node ){}
};

struct index_expression: public expression{
	expression::handle_t expr;
	expression::handle_t idxexpr;

	void accept( syntax_tree_visitor* visitor ){
		visitor->visit(*this);
	}

	void update(){
		expr->update();
		idxexpr->update();
	}
	index_expression(): expression( syntax_node_types::node ){}
};

struct call_expression: public expression{
	expression::handle_t expr;
	std::vector<expression::handle_t> params;

	void accept( syntax_tree_visitor* visitor ){
		visitor->visit(*this);
	}

	void update(){
		expr->update();
		for(size_t ipar = 0; ipar < params.size(); ++ipar){
			params[ipar]->update();
		}
	}

	call_expression(): expression( syntax_node_types::node ){}
};

struct member_expression: public expression{
	expression::handle_t expr;
	identifier::handle_t member_ident;
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}
	void update(){
		expr->update();
		member_ident->update();
	}

	member_expression(): expression( syntax_node_types::node ){}
};

#endif //SASL_SYNTAX_TREE_EXPRESSION_H