#include <sasl/include/parser/lexer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/lex.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/unordered_set.hpp>
#include <eflib/include/platform/boost_end.h>

namespace splex = boost::spirit::lex;

using sasl::common::lex_context;

using boost::make_shared;
using boost::shared_ptr;
using boost::unordered_map;
using boost::unordered_set;

using std::cout;
using std::endl;

BEGIN_NS_SASL_PARSER();

class attr_processor{
public:
	class state_translation_rule_adder{
	public:
		state_translation_rule_adder( attr_processor& proc )
			: proc(proc){}

		state_translation_rule_adder( state_translation_rule_adder const & rhs )
			: proc( rhs.proc ){}

		template <typename TokenDefT>
		state_translation_rule_adder& operator() ( TokenDefT const& tok_def, std::string const& on_state, std::string const& jump_to ){
			proc.add_state_translation_rule( tok_def, on_state, jump_to );
			return *this;
		}

	private:
		attr_processor& proc;
	};

	attr_processor( token_seq& attrs, std::vector<std::string> skippers, shared_ptr<lex_context> lex_ctxt )
		:attrs(attrs), ctxt(lex_ctxt)
	{
		BOOST_FOREACH( std::string const& skipper, skippers ){
			this->skippers.insert( skipper );
		}
	}

	template <typename TokenDefT>
	void add_state_translation_rule( TokenDefT const & tok_def, std::string const& on_state, std::string const& jump_to ){
		assert( state_translations.count( tok_def.id() ) == 0 );
		state_translations.insert(
			make_pair( make_pair(tok_def.id(), on_state), jump_to )
			);
	}

	state_translation_rule_adder add_state_translation_rule(){
		return state_translation_rule_adder( *this );
	}

	template <typename IteratorT, typename PassFlagT, typename IdT, typename ContextT>
	void operator ()(IteratorT& beg, IteratorT& end, PassFlagT& flag, IdT& id, ContextT& splexer_ctxt ){
		// process token
		std::string str(beg, end);

		// do skip
		std::string splexer_state( splexer_ctxt.get_state_name() );
		if( skippers.count( splexer_state ) == 0 ){
			ctxt->next( str );
			attrs.push_back( token::make(id, str, ctxt->line(), ctxt->column(), ctxt->file_name() ) );
		}

		// change state
		if( state_translations.count( make_pair( id, splexer_state ) ) > 0 ){
			splexer_ctxt.set_state_name( state_translations[ make_pair(id, splexer_state) ].c_str() );
		}
	}

private:
	boost::unordered_map< std::pair<size_t, std::string>, std::string > state_translations;
	unordered_set< std::string > skippers;
	token_seq& attrs;
	shared_ptr<lex_context> ctxt;
};

typedef boost::mpl::vector< std::string > token_attr_types;
typedef splex::lexertl::token< char const*, token_attr_types > token_t;
typedef splex::lexertl::actor_lexer< token_t > base_lexer_t;

struct lexer: public splex::lexer<base_lexer_t>{
	lexer( shared_ptr<token_def_table> defs, shared_ptr<attr_processor> proc );

	splex::token_def< std::string >
		lit_int, lit_float, lit_bool,
		
		kw_struct, kw_typedef,
		kw_break, kw_case, kw_continue, kw_return,
		kw_switch, kw_if, kw_else, kw_for, kw_while, kw_do,

		plus, minus, asterisk, slash,
		backslash, caret, ampersand, percent, equal, comma, colon, semicolon,
		dot, exclamation, question, squote, dquote, vertical, tilde,
		lparen, rparen, lbrace, rbrace, lsbracket, rsbracket, labracket, rabracket,

		ident,

		any_char,

		space,
		newline
		;

private:
	// unordered_map< std::string, splex::token_def<std::string> > defs;
	shared_ptr<token_def_table> defs;
};

#define TOKEN_DEF_EXPR( tok, match_rule ) \
	token_def_table::lazy_adder tok##_def_lazy_adder = defs->lazy_add( tok, std::string( #tok ) ); \
	tok = match_rule;

#define TOKEN_DEFS_I( r, data, i, elem ) BOOST_PP_IF( i, |, BOOST_PP_EMPTY() ) elem[*proc] 
#define TOKEN_DEFS( defs ) BOOST_PP_SEQ_FOR_EACH_I ( TOKEN_DEFS_I, 0, defs )

lexer::lexer( shared_ptr<token_def_table> defs, shared_ptr<attr_processor> proc ): defs(defs){
	this->self.add_pattern
		("SPACE", "[ \\t\\v\\f]+")
		("NEWLINE", "((\\r\\n?)|\\n)+")
		("NON_ZERO_DIGIT", "[1-9]")
		("DIGIT", "[0-9]")
		("SIGN", "[\\+\\-]")
		("DIGIT_SEQ", "([0-9]+)")
		("HEX_DIGIT", "[0-9a-fA-F]")
		("EXPONENT_PART", "((e|E)[+-]?[0-9]+)")
		("REAL_TYPE_SUFFIX", "[fFdD]")
		("INT_TYPE_SUFFIX", "([uULl]|([Uu][Ll)|([Ll][Uu]))")
		("PLUS", "\\+")
		("MINUS", "\\-")
		("ASTERISK", "\\*")
		("SLASH", "\\/")
		("DOT", "\\.")
		("BACKSLASH", "\\\\")
		("AMPERSAND", "\\^")
		("QUESTION", "\\?")
		("DQUOTE", "\\\"")
		("VERTICAL", "\\|")
		("LPAREN", "\\(")
		("RPAREN", "\\)")
		("LBRACE", "\\{")
		("RBRACE", "\\}")
		("LSBRACKET", "\\[")
		("RSBRACKET", "\\]")
		("LABRACKET", "\\<")
		("RABRACKET", "\\>")
		;

	TOKEN_DEF_EXPR( lit_int, "({DIGIT}+{INT_TYPE_SUFFIX}?)|(0x{HEX_DIGIT}+{INT_TYPE_SUFFIX}?)" );
	TOKEN_DEF_EXPR( lit_float, "({DIGIT_SEQ}?{DOT}{DIGIT_SEQ}{EXPONENT_PART}?{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{EXPONENT_PART}{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{REAL_TYPE_SUFFIX})" );
	TOKEN_DEF_EXPR( lit_bool, "(true)|(false)" );
	TOKEN_DEF_EXPR( ident, "[a-zA-Z_][0-9a-zA-Z_]*");

	TOKEN_DEF_EXPR( kw_struct, "struct" );
	TOKEN_DEF_EXPR( kw_typedef, "typedef" );
	TOKEN_DEF_EXPR( kw_break, "break" );
	TOKEN_DEF_EXPR( kw_continue, "continue" );
	TOKEN_DEF_EXPR( kw_case, "case" );
	TOKEN_DEF_EXPR( kw_return, "return" );
	TOKEN_DEF_EXPR( kw_switch, "switch" );
	TOKEN_DEF_EXPR( kw_else, "else" );
	TOKEN_DEF_EXPR( kw_for, "for" );
	TOKEN_DEF_EXPR( kw_if, "if" );
	TOKEN_DEF_EXPR( kw_while, "while" );
	TOKEN_DEF_EXPR( kw_do, "do" );

	TOKEN_DEF_EXPR( plus, "{PLUS}" );
	TOKEN_DEF_EXPR( minus, "{MINUS}" );
	TOKEN_DEF_EXPR( asterisk, "{ASTERISK}" );
	TOKEN_DEF_EXPR( slash, "{SLASH}");
	TOKEN_DEF_EXPR( backslash, "{BACKSLASH}" );
	TOKEN_DEF_EXPR( caret, "{AMPERSAND}" );
	TOKEN_DEF_EXPR( ampersand, "&" );
	TOKEN_DEF_EXPR( percent, "%" );
	TOKEN_DEF_EXPR( equal, "=" );
	TOKEN_DEF_EXPR( comma, "," );
	TOKEN_DEF_EXPR( colon, ":" );
	TOKEN_DEF_EXPR( semicolon, ";" );
	TOKEN_DEF_EXPR( dot, "{DOT}" );
	TOKEN_DEF_EXPR( exclamation, "!" );
	TOKEN_DEF_EXPR( question, "{QUESTION}" );
	TOKEN_DEF_EXPR( squote, "'" );
	TOKEN_DEF_EXPR( dquote, "{DQUOTE}" );
	TOKEN_DEF_EXPR( vertical, "{VERTICAL}" );
	TOKEN_DEF_EXPR( tilde, "~" );

	TOKEN_DEF_EXPR( lparen, "{LPAREN}" );
	TOKEN_DEF_EXPR( rparen, "{RPAREN}" );
	TOKEN_DEF_EXPR( lbrace, "{LBRACE}" );
	TOKEN_DEF_EXPR( rbrace, "{RBRACE}" );
	TOKEN_DEF_EXPR( lsbracket, "{LSBRACKET}" );
	TOKEN_DEF_EXPR( rsbracket, "{RSBRACKET}" );
	TOKEN_DEF_EXPR( labracket, "{LABRACKET}" );
	TOKEN_DEF_EXPR( rabracket, "{RABRACKET}" );

	TOKEN_DEF_EXPR( any_char, "." );

	TOKEN_DEF_EXPR( space, "{SPACE}" );
	TOKEN_DEF_EXPR( newline, "{NEWLINE}" );
	
	this->self = TOKEN_DEFS(
		(lit_int)(lit_float)(lit_bool)
		(kw_struct) (kw_typedef)
		(kw_break) (kw_case) (kw_continue) (kw_return)
		(kw_if) (kw_else) (kw_for) (kw_switch) (kw_while) (kw_do)
		(semicolon)
		(ident)
		);

	this->self("SKIPPED") = TOKEN_DEFS( (space) (newline) );
}

bool tokenize(
	/*IN*/ std::string const& code,
	/*IN*/ shared_ptr< sasl::common::lex_context > ctxt,
	/*IN*/ std::vector<std::string> skippers,
	/*OUT*/ shared_ptr<token_def_table> defs,
	/*OUT*/ token_seq& cont
	)
{
	const char* lex_first = &code[0];
	const char* lex_last = &code[0] + code.size();

	shared_ptr<attr_processor> proc( new attr_processor(cont, skippers, ctxt) );
	lexer lxr( defs, proc );

	// Try to use all lex state for tokenize character sequence.
	const size_t tok_states_count = 2;
	const char* tok_states[tok_states_count] = {NULL, "SKIPPED"};

	int toked_state = 0; // 0 is no result, 1 is succeed, 2 is failed.
	int i_state = 0;
	while( lex_first != lex_last && toked_state == 0 ){

		const char* next_lex_first = lex_first;

		tokenize( next_lex_first, lex_last, lxr, tok_states[i_state] );

		// next state.
		i_state = (++i_state) % tok_states_count;

		if( next_lex_first == lex_last ){
			toked_state = 1;
			break;
		}

		if( next_lex_first == lex_first ){
			toked_state = 2;
			break;
		}

		lex_first = next_lex_first;
	}

	bool tokenize_succeed = (toked_state == 1);
	return tokenize_succeed;
}

END_NS_SASL_PARSER();