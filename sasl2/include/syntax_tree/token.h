#ifndef SASL_SYNTAX_TREE_TOKEN_H
#define SASL_SYNTAX_TREE_TOKEN_H

#include "node.h"

struct token_attr: public terminal<token_attr>{
	typedef token_attr_handle handle_t;

	token_attr( const token_attr_handle& )
		: terminal<token_attr>( syntax_node_types::token, handle_t() ) {}

	template< typename IteratorT >
	token_attr( const IteratorT& first, const IteratorT& last ):
		: terminal<token_attr>( syntax_node_types::token, handle_t() ){
	}

	token_attr( 
		const std::string& lit = std::string(), 
		size_t line = 0, size_t column = 0, 
		const std::string& file_name = std::string("undefined") )
		
		:lit(lit), 
		line(line), column(column),
		file_name(file_name),
		terminal<token_attr>( syntax_node_types::token, handle_t() )
	{
	}

	void update() {}

	std::string lit;
	std::size_t line;
	std::size_t column;
	std::string file_name;
};

#endif