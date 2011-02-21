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

class token_def_table{
public:
	class lazy_adder{
	public:
		lazy_adder( token_def_table& tbl, size_t const* id, std::string const& name )
			: p_id(id), name(name), tbl(tbl){}

		lazy_adder( lazy_adder const& rhs )
			: p_id(rhs.p_id), name(rhs.name), tbl(rhs.tbl){}

		~lazy_adder(){
			tbl.add( *p_id, name );
		}
	private:
		token_def_table& tbl;
		size_t const * p_id;
		std::string name;
	};

	template <typename TokenDefT>
	lazy_adder lazy_add( TokenDefT const& def, std::string const& name ){
		return lazy_adder(*this, &def.id(), name);
	}

	template <typename TokenDefT>
	void add( TokenDefT const& def, std::string const& name ){
		add( def.id(), name );
	}

	void add( size_t id, std::string const& name ){
		id_to_name.insert( make_pair(id, name) );
		name_to_id.insert( make_pair(name, id) );
	}

	std::string const & operator []( size_t id ) const{
		return id_to_name.at(id);
	}

	size_t operator []( std::string const& name ) const{
		return name_to_id.at(name);
	}

private:
	boost::unordered_map< size_t, std::string > id_to_name;
	boost::unordered_map< std::string, size_t > name_to_id;
};

class token;
typedef boost::shared_ptr<token> token_ptr;
typedef std::vector< token_ptr > token_seq;
typedef token_seq::iterator token_iterator;

//class lexer_impl;
//class attr_processor;
//
//class lexer{
//	lexer( boost::shared_ptr<attr_processor> proc );
//
//	class token_definer{
//		token_definer( lexer_impl& owner );
//	private:
//		lexer& owner;
//	};
//
//	class pattern_adder{
//		pattern_adder( lexer_impl& owner );
//	private:
//		lexer& owner;
//	};
//
//	class token_adder{
//		token_adder( lexer_impl& owner, char const* state );
//	private:
//		lexer& owner;
//	};
//
//	void add_token_definition( std::string const& name, std::string const& patterndef );
//	void add_pattern( std::string const& name, std::string const& patterndef );
//	void add_token( const char* state, std::string const& name );
//
//private:
//	shared_ptr<lexer_impl> impl;
//};

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
	/*IN*/ boost::shared_ptr< ::sasl::common::lex_context > ctxt,
	/*IN*/ std::vector<std::string> skippers,
	/*OUT*/ boost::shared_ptr<token_def_table> defs,
	/*OUT*/ token_seq& cont
	);

END_NS_SASL_PARSER();

#endif