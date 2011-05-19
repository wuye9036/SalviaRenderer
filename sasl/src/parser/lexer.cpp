#include <sasl/include/parser/lexer.h>

#include <sasl/include/common/token.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/lex.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/unordered_set.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>

namespace splex = boost::spirit::lex;

using sasl::common::lex_context;
using sasl::common::token_t;

using boost::make_shared;
using boost::shared_ptr;
using boost::unordered_map;
using boost::unordered_set;

using std::cout;
using std::endl;
using std::vector;

BEGIN_NS_SASL_PARSER();

class shared_data{
public:
	shared_data(): attrs(NULL){}
	boost::unordered_map< std::pair<size_t, std::string>, std::string > state_translations;
	unordered_set< std::string > skippers;
	unordered_set< std::string > init_states;
	token_seq* attrs;
	shared_ptr<lex_context> ctxt;
};

class attr_processor{
public:
	attr_processor(){
		data = make_shared<shared_data>();
	}

	attr_processor( attr_processor const& rhs ): data(rhs.data){
	}

	void output( token_seq& seq ){
		data->attrs = &seq;
	}

	void context( shared_ptr< lex_context > ctxt ){
		data->ctxt = ctxt;
	}

	void add_skipper( std::string const& s ){
		data->skippers.insert(s);
	}
	
	vector<std::string> get_skippers() const{
		return vector<std::string>( data->skippers.begin(), data->skippers.end() );
	}

	void add_init_states( std::string const& s ){
		data->init_states.insert(s);
	}

	vector<std::string> get_init_states() const{
		return vector<std::string>( data->init_states.begin(), data->init_states.end() );
	}

	void add_state_translation_rule( size_t tok_def_id, std::string const& on_state, std::string const& jump_to ){
		std::pair<size_t, std::string> key_pair = make_pair(tok_def_id, on_state);
		assert( data->state_translations.count( key_pair ) == 0 );
		data->state_translations.insert(
			make_pair( key_pair, jump_to )
			);
	}

	template <typename IteratorT, typename PassFlagT, typename IdT, typename ContextT>
	void operator ()(IteratorT& beg, IteratorT& end, PassFlagT& /*flag*/, IdT& id, ContextT& splexer_ctxt ){
		// process token
		std::string str(beg, end);

		// do skip
		std::string splexer_state( splexer_ctxt.get_state_name() );
		if( data->skippers.count( splexer_state ) == 0 ){
			data->ctxt->next( str );
			data->attrs->push_back( token_t::make(id, str, data->ctxt->line(), data->ctxt->column(), data->ctxt->file_name() ) );
		}

		// change state
		if( data->state_translations.count( make_pair( id, splexer_state ) ) > 0 ){
			splexer_ctxt.set_state_name( data->state_translations[ make_pair(id, splexer_state) ].c_str() );
		}
	}

private:
	attr_processor& operator = ( attr_processor const & );
	shared_ptr<shared_data> data;
};

typedef boost::mpl::vector< std::string > token_types;
typedef splex::lexertl::token< char const*, token_types > splex_token;
typedef splex::lexertl::actor_lexer< splex_token > base_lexer_t;

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
	shared_ptr<lexer_impl> impl = owner.get_impl();
	splex::token_def< std::string >& def = impl->defs[name];
	impl->self(state).define(def[*(impl->proc)]);
	impl->ids.insert( make_pair( def.id(), name ) );
	return *this;
}

lexer::token_adder const& lexer::token_adder::operator()( std::string const& name, std::string const& jump_to ) const
{
	(*this)( name );
	shared_ptr<lexer_impl> impl = owner.get_impl();
	impl->proc->add_state_translation_rule( impl->defs[name].id(), state, jump_to );
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

lexer::init_states_adder::init_states_adder( lexer& owner ): owner(owner)
{
}

lexer::init_states_adder::init_states_adder( init_states_adder const& rhs ): owner(rhs.owner)
{
}

lexer::init_states_adder const& lexer::init_states_adder::operator()( std::string const& name ) const{
	owner.get_impl()->proc->add_init_states(name);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// lexer members
lexer::lexer()
{
	shared_ptr<attr_processor> proc( new attr_processor() );
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

lexer::init_states_adder lexer::init_states( std::string const& s )
{
	return init_states_adder(*this)(s);
}

std::string const& lexer::get_name( size_t id ){
	return impl->ids[id];
}

size_t lexer::get_id( std::string const& name ){
	return impl->defs[name].id();
}

bool lexer::tokenize( /*INPUTS*/ std::string const& code, shared_ptr<lex_context> ctxt, /*OUTPUT*/ token_seq& seq )
{
	impl->proc->output( seq );
	impl->proc->context( ctxt );

	const char* lex_first = &code[0];
	const char* lex_last = &code[0] + code.size();

	// Try to use all lex state for tokenize character sequence.
	std::vector<std::string> tok_states = impl->proc->get_init_states();
	EFLIB_ASSERT_AND_IF( !tok_states.empty(), "Initial state set should not be empty." ){
		return false;
	}

	size_t tok_states_count = tok_states.size();

	int toked_state = 0; // 0 is no result, 1 is succeed, 2 is failed.
	int i_state = 0;
	int failed_count = 0;
	while( lex_first != lex_last && toked_state == 0 ){
		// Use current state, and match as long as possible.
		const char* next_lex_first = lex_first;
		splex::tokenize( next_lex_first, lex_last, *impl, tok_states[i_state].c_str() );

		// All was matched, return success(1).
		if( next_lex_first == lex_last ){
			toked_state = 1;
			break;
		}

		// If failed, add failed count.
		if( next_lex_first == lex_first ){
			++failed_count;
		} else {
			failed_count = 0;
		}

		// If all states was tried and failed, it is really failed.
		if( failed_count == tok_states_count ){
			toked_state = 2;
			break;
		}

		// Otherwise, try next state.
		i_state = (++i_state) % tok_states_count;

		lex_first = next_lex_first;
	}

	bool tokenize_succeed = (toked_state != 2);
	return tokenize_succeed;
}

shared_ptr<lexer_impl> lexer::get_impl() const
{
	return impl;
}

bool lexer::begin_incremental()
{
	return true;
}

bool lexer::incremental_tokenize( std::string const& word, boost::shared_ptr<sasl::common::lex_context> ctxt, token_seq& seq )
{
	return tokenize(word, ctxt, seq);
}

bool lexer::end_incremental()
{
	return true;
}

END_NS_SASL_PARSER();