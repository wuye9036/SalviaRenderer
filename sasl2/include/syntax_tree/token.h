#ifndef SASL_SYNTAX_TREE_TOKEN_H
#define SASL_SYNTAX_TREE_TOKEN_H

struct token_attr{
	template< typename IteratorT >
	token_attr( const IteratorT& first, const IteratorT& last ){
		lit.assign( first, last );
		std::cout << "Literal: " << lit << " accepted." << endl;
	}
	token_attr( const std::string& lit = std::string(), 
		size_t line = 0, size_t column = 0, 
		const std::string& file_name = std::string("undefined") ):
	lit(lit), line(line), column(column), file_name(file_name)
	{
	}

	std::string lit;
	std::size_t line;
	std::size_t column;
	std::string file_name;
};

#endif