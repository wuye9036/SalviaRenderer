#ifndef SASL_SYNTAX_TREE_TOKEN_H
#define SASL_SYNTAX_TREE_TOKEN_H

#include <string>
#include <boost/smart_ptr.hpp>

struct token_attr{
	typedef boost::shared_ptr<token_attr> handle_t;

	token_attr()
		: file_name("undefined"), column(0), line(0), lit("UNINITIALIZED_VALUE"){}
	token_attr( const token_attr& rhs )
		: file_name( rhs.file_name ), column(rhs.column), line(rhs.line), lit(rhs.lit){}
	template< typename IteratorT > token_attr( const IteratorT& first, const IteratorT& last ){}

	token_attr& operator = ( const token_attr& rhs){
		file_name = rhs.file_name;
		column = rhs.column;
		line = rhs.line;
		lit = rhs.lit;	
		return *this;
	}

	std::string lit;
	std::size_t line;
	std::size_t column;
	std::string file_name;
};

#endif