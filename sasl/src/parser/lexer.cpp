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
using std::vector;

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

	class skipper_adder{
	public:
		skipper_adder( attr_processor& proc )
			: proc(proc){}

		skipper_adder( skipper_adder const & rhs )
			: proc( rhs.proc ){}

		skipper_adder& operator()( std::string const& s ){
			proc.skippers.insert(s);
			return *this;
		}
	private:
		attr_processor& proc;
	};

	attr_processor( token_seq& attrs, shared_ptr<lex_context> lex_ctxt )
		:attrs(attrs), ctxt(lex_ctxt)
	{
	}

	skipper_adder add_skipper( std::string const& s ){
		return skipper_adder(*this)(s);
	}
	
	vector<std::string> get_skippers() const{
		return vector<std::string>( skippers.begin(), skippers.end() );
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

struct lexer_impl: public splex::lexer<base_lexer_t>{
	lexer_impl( shared_ptr<attr_processor> proc );

	unordered_map< std::string, splex::token_def<std::string> > defs;
	unordered_map< size_t, std::string > ids;
	shared_ptr<attr_processor> proc;
};

lexer_impl::lexer_impl( shared_ptr<attr_processor> proc ): proc(proc){
}

//////////////////////////////////////////////////////////////////////////
// adders
lexer::token_definer::token_definer( lexer& owner ):owner(owner)
{
}

lexer::token_definer::token_definer( token_definer const& rhs ):owner(rhs.owner)
{
}

lexer::token_definer const& lexer::token_definer::operator()( std::string const& name, std::string const& patterndef ) const
{
	owner.get_impl()->defs[name] = patterndef;
	return *this;
}

lexer::pattern_adder::pattern_adder( lexer& owner ):owner(owner)
{
}

lexer::pattern_adder::pattern_adder( pattern_adder const& rhs ):owner(rhs.owner)
{
}

lexer::pattern_adder const& lexer::pattern_adder::operator()( std::string const& name, std::string const& patterndef ) const
{
	owner.get_impl()->self.add_pattern( name, patterndef );
	return *this;
}

lexer::token_adder::token_adder( lexer& owner, char const* state ): owner(owner), state(state)
{
}

lexer::token_adder::token_adder( token_adder const& rhs ):owner(rhs.owner), state(rhs.state)
{
}

lexer::token_adder const& lexer::token_adder::operator()( std::string const& name ) const
{
	owner.get_impl()->self(state) += (owner.get_impl()->defs[name])[*(owner.get_impl()->proc)];
	owner.get_impl()->ids.insert( make_pair( owner.get_impl()->defs[name].id(), name ) );
	return *this;
}


lexer::skippers_adder::skippers_adder( lexer& owner ) : owner(owner)
{
}

lexer::skippers_adder::skippers_adder( skippers_adder const& rhs ):owner(rhs.owner)
{
}

lexer::skippers_adder const& lexer::skippers_adder::operator()( std::string const& name ) const
{
	owner.get_impl()->proc->add_skipper(name);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// lexer members
lexer::lexer( token_seq& seq, shared_ptr<sasl::common::lex_context> ctxt )
{
	shared_ptr<attr_processor> proc( new attr_processor(seq, ctxt) );
	impl = boost::make_shared<lexer_impl>( proc );
}

lexer::token_definer lexer::define_tokens( std::string const& name, std::string const& patterndef )
{
	return token_definer(*this)(name, patterndef);
}

lexer::pattern_adder lexer::add_pattern( std::string const& name, std::string const& patterndef )
{
	return pattern_adder(*this)(name, patterndef);
}

lexer::token_adder lexer::add_token( const char* state )
{
	return token_adder(*this, state);
}

lexer::skippers_adder lexer::skippers( std::string const& s )
{
	return skippers_adder(*this)(s);
}

std::string const& lexer::get_name( size_t id ){
	return impl->ids[id];
}

size_t lexer::get_id( std::string const& name ){
	return impl->defs[name].id();
}

shared_ptr<lexer_impl> lexer::get_impl() const
{
	return impl;
}

//////////////////////////////////////////////////////////////////////////
// tokenize API

bool tokenize(
	/*IN*/ std::string const& code,
	/*IN*/ lexer const& lxr,
	/*OUT*/ token_seq& cont
	)
{
	const char* lex_first = &code[0];
	const char* lex_last = &code[0] + code.size();

	// Try to use all lex state for tokenize character sequence.
	std::vector<std::string> tok_states = lxr.get_impl()->proc->get_skippers();
	tok_states.push_back( std::string("INITIAL") );

	size_t tok_states_count = tok_states.size();

	int toked_state = 0; // 0 is no result, 1 is succeed, 2 is failed.
	int i_state = 0;
	while( lex_first != lex_last && toked_state == 0 ){

		const char* next_lex_first = lex_first;

		splex::tokenize( next_lex_first, lex_last, *(lxr.get_impl()), tok_states[i_state].c_str() );

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