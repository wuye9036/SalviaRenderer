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

namespace sasl{
	namespace common{
		struct token_t;
	}
}

BEGIN_NS_SASL_PARSER();

typedef boost::shared_ptr< sasl::common::token_t > token_ptr;
typedef std::vector< token_ptr > token_seq;
typedef token_seq::iterator token_iterator;

struct lexer_impl;

class lexer{
public:
	lexer();

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

	bool tokenize(
		/*INPUTS*/ std::string const& code, boost::shared_ptr<sasl::common::lex_context> ctxt,
		/*OUTPUT*/ token_seq& seq
		);
private:
	boost::shared_ptr<lexer_impl> impl;
};

END_NS_SASL_PARSER();

#endif