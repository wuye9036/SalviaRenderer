#ifndef SASL_PARSER_LEXER_H
#define SASL_PARSER_LEXER_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/lex_context.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>
#include <string>
#include <vector>

BEGIN_NS_SASL_PARSER();

class token;
typedef boost::shared_ptr<token> token_ptr;
typedef std::vector< token_ptr > token_seq;
typedef token_seq::iterator token_iterator;

struct lexer_impl;

class lexer{
public:
	lexer( token_seq& seq, boost::shared_ptr<sasl::common::lex_context> ctxt );

	class token_definer{
	public:
		token_definer( lexer& owner );
		token_definer( token_definer const& rhs );
		token_definer const& operator()( std::string const& name, std::string const& patterndef ) const;
	private:
		token_definer& operator = ( token_definer const& rhs );
		lexer& owner;
	};

	class pattern_adder{
	public:
		pattern_adder( lexer& owner );
		pattern_adder( pattern_adder const& rhs );
		pattern_adder const& operator()( std::string const& name, std::string const& patterndef ) const;
	private:
		pattern_adder& operator = ( pattern_adder const& rhs );
		lexer& owner;
	};

	class token_adder{
	public:
		token_adder( lexer& owner, char const* state );
		token_adder( token_adder const& rhs );
		token_adder const& operator()( std::string const& name ) const;
	private:
		token_adder& operator = ( token_adder const& rhs );
		lexer& owner;
		char const* state;
	};

	class skippers_adder{
	public:
		skippers_adder( lexer& owner );
		skippers_adder( skippers_adder const& rhs );
		skippers_adder const& operator()( std::string const& name ) const;
	private:
		skippers_adder& operator = ( skippers_adder const& rhs );
		lexer& owner;
	};

	token_definer define_tokens( std::string const& name, std::string const& patterndef );
	pattern_adder add_pattern( std::string const& name, std::string const& patterndef );
	token_adder add_token( const char* state );

	skippers_adder skippers( std::string const& s );

	std::string const& get_name( size_t id );
	size_t get_id( std::string const& name );

	boost::shared_ptr<lexer_impl> get_impl() const;

private:
	boost::shared_ptr<lexer_impl> impl;
};

class token{
public:	
	static token_ptr make( size_t id, std::string const & lit, size_t line, size_t col, std::string const& fname ){
		return token_ptr( new token(id, lit, line, col, fname) );
	}

	size_t id() const{
		return tok_id;
	}

	size_t line() const;
	size_t col() const;
	std::string const& file_name() const;
	std::string const& str() const{
		return tok_lit;
	}
private:
	token& operator = ( token const& );
	token( const token& );

	token( size_t id, std::string const & lit, size_t line, size_t col, std::string const& fname ):
	tok_id(id), tok_line(line), tok_col(col), tok_fname(fname), tok_lit(lit)
	{
	}

	size_t tok_id;
	size_t tok_line;
	size_t tok_col;
	std::string tok_fname;
	std::string tok_lit;
};

bool tokenize(
	/*IN*/ std::string const& code,
	/*IN*/ lexer const& lxr,
	/*OUT*/ token_seq& cont
	);

END_NS_SASL_PARSER();

#endif