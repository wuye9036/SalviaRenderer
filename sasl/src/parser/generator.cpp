#include <sasl/include/parser/generator.h>

#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/diags.h>
#include <sasl/include/parser/error_handlers.h>
#include <sasl/include/common/token.h>
#include <sasl/include/common/diag_chat.h>
#include <sasl/include/common/diag_item.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/preprocessor.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>
#include <iostream>
#include <fstream>

using sasl::common::diag_template;
using sasl::common::diag_item;
using sasl::common::diag_chat;
using sasl::common::token_t;

using boost::shared_ptr;
using boost::make_shared;

using std::vector;

using std::cout;
using std::endl;
using std::string;

BEGIN_NS_SASL_PARSER();

parse_results::parse_results(): tag(-1){}

parse_results::parse_results( int t ): tag(t) {}

namespace parse_result_values
{
	int const recover_failed_offset = 4;
	int const recover_failed_mask = 0xF0;
	int const expected_offset = 0;
	int const expected_mask = 0x0F;
	int const expected = 0x01;
	int const succeed	= 0;
	int const recovered	= 0x1 << recover_failed_offset;
	int const failed	= 0x2 << recover_failed_offset;

	int const expected_failed			= expected | failed;
	int const recovered_expected_failed	= expected | recovered;
	
};


parse_results const parse_results::succeed					(parse_result_values::succeed);
parse_results const parse_results::recovered				(parse_result_values::recovered);
parse_results const parse_results::failed					(parse_result_values::failed);
parse_results const parse_results::expected_failed			(parse_result_values::expected_failed);
parse_results const parse_results::recovered_expected_failed(parse_result_values::recovered_expected_failed);

bool parse_results::is_succeed    ()					const { return tag == succeed.tag; }
bool parse_results::is_failed     ()					const { return tag == failed.tag; }
bool parse_results::is_recovered  ()					const { return tag == recovered.tag; }
bool parse_results::is_expected_failed()				const { return tag == expected_failed.tag; }
bool parse_results::is_recovered_expected_failed()		const { return tag == recovered_expected_failed.tag; }

bool parse_results::is_continuable()					const { return (tag & parse_result_values::recover_failed_mask) != parse_result_values::failed; }
bool parse_results::is_expected_failed_or_recovered()	const { return (tag & parse_result_values::expected_mask) == parse_result_values::expected; }

parse_results parse_results::worse( parse_results const& l, parse_results const& r )
{
	return l.tag > r.tag ? l : r;
}

parse_results parse_results::better( parse_results const& l, parse_results const& r )
{
	return l.tag < r.tag ? l : r;
}

bool parse_results::worse_than( parse_results const& v ) const
{
	return tag > v.tag;
}

bool parse_results::better_than( parse_results const& v ) const
{
	return tag < v.tag;
}

parse_results parse_results::recover( parse_results const& v )
{
	if( v.is_expected_failed() ){
		return recovered_expected_failed;
	}
	return recovered;
}


//////////////////////////////////////////////////////////////////////////
// Exceptions
expectation_failure::expectation_failure( token_iterator iter, parser const* p ): iter(iter), p(p)
{
	rule_wrapper const* r = dynamic_cast<rule_wrapper const*>(p);
	if( r ){
		what_str = str(
			boost::format("can't match expected rule \"%s\"") % r->name()
			);
	} else {
		// TODO: For token & default

#if	defined( EFLIB_CPU_X64 )
		what_str = str(
			boost::format( "can't match unknown parser at 0x%016p") % p
			);
#else
		what_str = str(
			boost::format( "can't match unknown parser at 0x%08p") % p
			);
#endif
	}
}

parser const* expectation_failure::get_parser(){ return p; }
const char* expectation_failure::what() const {	return what_str.c_str(); }

//////////////////////////////////////////////////////////////////////////
// Attributes.
attribute::attribute() :rid(-1){}
attribute::~attribute(){}
intptr_t attribute::rule_id() const{ return rid; }
void attribute::rule_id( intptr_t id ){ rid = id; }

void attribute::token_range( token_ptr const& beg, token_ptr const& end )
{
	tok_beg = beg;
	tok_end = end;
}

token_ptr attribute::token_beg() const
{
	return tok_beg;
}

token_ptr attribute::token_end() const
{
	return tok_end;
}

shared_ptr<attribute> terminal_attribute::child( int /*idx*/ ) const{
	assert(!"Terminate attribute has no child.");
	return shared_ptr<attribute>();
}

size_t terminal_attribute::child_size() const{
	return 0;
}

shared_ptr<attribute> sequence_attribute::child( int idx ) const{
	if( attrs.empty() ){
		assert( idx == 0 );
		return shared_ptr<attribute>();
	}

	EFLIB_ASSERT_AND_IF( 0 <= idx && idx < static_cast<int>( attrs.size() ), "" ){
		return shared_ptr<attribute>();
	}
	return attrs[idx];
}

size_t sequence_attribute::child_size() const{
	return attrs.size();
}

selector_attribute::selector_attribute() : selected_idx(-1){}
shared_ptr<attribute> selector_attribute::child( int idx ) const{
	EFLIB_ASSERT_AND_IF( idx == 0, "" ){
		return shared_ptr<attribute>();
	}
	return attr;
}

size_t selector_attribute::child_size() const
{
	return selected_idx == -1 ? 0 : 1;
}

shared_ptr<attribute> queuer_attribute::child( int idx ) const{
	EFLIB_ASSERT_AND_IF( 0 <= idx && idx < static_cast<int>( attrs.size() ), "" ){
		return shared_ptr<attribute>();
	}
	return attrs[idx];
}

size_t queuer_attribute::child_size() const{
	return attrs.size();
}

//////////////////////////////////////////////////////////////////////////
// Parsers

parser::parser(): expected(false){}
bool parser::is_expected() const{ return expected; }
void parser::is_expected( bool v ){ expected = v; }

error_catcher parser::operator[]( error_handler on_err )
{
	return error_catcher( clone(), on_err );
}

terminal::terminal( size_t tok_id, std::string const& desc ) :tok_id(tok_id), desc(desc){}

terminal::terminal( terminal const& rhs ) :tok_id(rhs.tok_id), desc(rhs.desc){}

parse_results terminal::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr, diag_chat* diags ) const
{
	shared_ptr<terminal_attribute> ret = make_shared<terminal_attribute>();
	ret->tok = *iter;
	ret->token_range( *iter, *iter );
	attr = ret;

	if ( iter == end ){
		return parse_results::failed;
	}

	if( (*iter)->id == tok_id ){
		token_iterator current_iter = iter;
		++iter;
		ret->token_range( *current_iter, *iter );
		return parse_results::succeed;
	}

	diags->report( unmatched_token )->token_range(**iter, **iter)->p( (*iter)->str );
	return parse_results::failed;
}

shared_ptr<parser> terminal::clone() const
{
	return make_shared<terminal>(*this);
}

string const& terminal::get_desc() const
{
	return desc;
}

size_t const repeater::unlimited = std::numeric_limits<size_t>::max();

repeater::repeater( size_t lower_bound, size_t upper_bound, shared_ptr<parser> expr ) : lower_bound(lower_bound), upper_bound(upper_bound), expr(expr){}

repeater::repeater( repeater const& rhs ) : lower_bound(rhs.lower_bound), upper_bound(rhs.upper_bound), expr(rhs.expr){}

parse_results repeater::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr, diag_chat* diags ) const
{
	token_iterator iter_beg = iter;

	vector< shared_ptr<diag_chat> > children_diags;
	
	shared_ptr<sequence_attribute> seq_attr = make_shared<sequence_attribute>();
	attr = seq_attr;
	attr->token_range( *iter_beg, *iter_beg );

	parse_results final_result = parse_results::succeed;

	for(size_t matched_count = 0;; ++matched_count )
	{
		shared_ptr<attribute> out;
		shared_ptr<diag_chat> children_diags = diag_chat::create();

		token_iterator child_start = iter;
		parse_results result = expr->parse( iter, end, out, children_diags.get() );
		
		// Child matched succeed.
		if( result.is_succeed() ) {
			out->token_range( *iter_beg, *iter );
			seq_attr->attrs.push_back(out);
			if( matched_count == upper_bound ){ break; }
			continue;
		}
		
		// Repeater matched failed.
		if( matched_count < lower_bound ){
			iter = iter_beg;
			seq_attr->token_range( *iter_beg, *iter_beg );
			diag_chat::merge( diags, children_diags.get(), true );
			return parse_results::failed;
		}

		// if normal failed, maybe it's successful, just recover iterator and return.
		if( result.is_expected_failed_or_recovered() )
		{
			diag_chat::merge( diags, children_diags.get(), true );
			final_result = parse_results::worse( result, final_result );
			continue;
		}
		else
		{
			iter = child_start;
			return final_result;
		}
	}

	assert(false);
	return final_result;
}

shared_ptr<parser> repeater::clone() const
{
	return make_shared<repeater>(*this);
}

selector::selector(){}

selector::selector( selector const& rhs ) : slc_branches(rhs.slc_branches){}

selector& selector::add_branch( shared_ptr<parser> p )
{
	slc_branches.push_back(p);
	return *this;
}

std::vector< shared_ptr<parser> > const& selector::branches() const
{
	return slc_branches;
}

parse_results selector::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr, diag_chat* diags ) const
{
	shared_ptr<selector_attribute> slc_attr = make_shared<selector_attribute>();
	slc_attr->token_range(*iter, *iter);
	vector< shared_ptr<diag_chat> > branch_diags;
	vector< parse_results >			results;
	vector< shared_ptr<attribute> >	attrs;
	vector< token_iterator >		iters;
	int idx = 0;

	// Visit branch and return succeed branch.
	BOOST_FOREACH( shared_ptr<parser> const& p, branches() )
	{
		token_iterator start_iter = iter;

		branch_diags.push_back( diag_chat::create() );
		shared_ptr<attribute> branch_attr;
		parse_results branch_result = p->parse( iter, end, branch_attr, branch_diags.back().get() );
		results.push_back( branch_result );
		attrs.push_back( branch_attr );
		iters.push_back( iter );

		if( branch_result.is_succeed() ){
			slc_attr->attr = branch_attr;
			slc_attr->token_range( branch_attr->token_beg(), branch_attr->token_end() );
			slc_attr->selected_idx = idx;
			attr = slc_attr;
			return parse_results::succeed;
		} else {
			iter = start_iter;
		}
		++idx;
	}

	assert( !branch_diags.empty() );

	// All branch matched failed, return the best one.
	shared_ptr<diag_chat>	least_error_branch_diags = branch_diags[0];
	parse_results			final_result = results[0];
	slc_attr->attr = attrs[0];
	slc_attr->selected_idx = 0;
	iter = iters[0];

	for ( size_t i_result = 1; i_result < branch_diags.size(); ++i_result )
	{
		parse_results branch_result = results[i_result];
		shared_ptr<diag_chat> branch_chat = branch_diags[i_result];
		if( branch_result.is_recovered_expected_failed() && !final_result.is_recovered_expected_failed()
			|| std::distance( iter, iters[i_result] ) > 0 )
		{
			least_error_branch_diags = branch_chat;
			final_result = branch_result;
			slc_attr->attr = attrs[i_result];
			slc_attr->token_range( attrs[i_result]->token_beg(), attrs[i_result]->token_end() );
			slc_attr->selected_idx = i_result;
			iter = iters[i_result];
		}
	}

	attr = slc_attr;
	diags->merge( diags, least_error_branch_diags.get(), true );

	return final_result;
}

shared_ptr<parser> selector::clone() const
{
	shared_ptr<selector> ret = make_shared<selector>();
	BOOST_FOREACH( shared_ptr<parser> p, branches() )
	{
		ret->add_branch(p);
	}
	return ret;
}


queuer::queuer(){}

queuer::queuer( queuer const& rhs ) :exprlst(rhs.exprlst){}

queuer& queuer::append( shared_ptr<parser> p, bool is_expected )
{
	p->is_expected(is_expected);
	
	// Add default error handler
	if( is_expected )
	{
		terminal* term = dynamic_cast<terminal*>( p.get() );
		if( term )
		{ 
			shared_ptr<error_catcher> err_catcher( new error_catcher( p, get_expected_failed_handler(term->get_desc()) ) );
			exprlst.push_back( err_catcher );
			return *this;
		}
		else 
		{
			rule_wrapper* rule_wrp = dynamic_cast<rule_wrapper*>(p.get());
			if( rule_wrp )
			{
				parser const* inner_p = rule_wrp->get_rule()->get_parser();
				terminal const* inner_term = dynamic_cast<terminal const*>(inner_p);
				if( inner_term )
				{
					shared_ptr<error_catcher> err_catcher( new error_catcher( p, get_expected_failed_handler(inner_term->get_desc()) ) );
					exprlst.push_back( err_catcher );
					return *this;
				}
				else
				{
					shared_ptr<error_catcher> err_catcher( new error_catcher( p, get_expected_failed_handler( rule_wrp->get_rule()->name() ) ) );
					exprlst.push_back( err_catcher );
					return *this;
				}
			}
		}
	}
	
	exprlst.push_back(p);
	return *this;
}

std::vector< shared_ptr<parser> > const& queuer::exprs() const
{
	return exprlst;
}

parse_results queuer::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr, diag_chat* diags ) const
{
	token_iterator iter_beg = iter;

	shared_ptr<queuer_attribute> ret = make_shared<queuer_attribute>();
	ret->token_range( *iter, *iter );
	shared_ptr<attribute> out;
	parse_results final_result = parse_results::succeed;

	BOOST_FOREACH( shared_ptr<parser> p, exprlst ){
		out.reset();
		token_iterator cur_iter = iter;
		
		parse_results result = p->parse(iter, end, out, diags);

		ret->attrs.push_back(out);
		ret->token_range( *iter_beg, out->token_end() );
		if( !result.is_continuable() ){
			attr = ret;
			if( p->is_expected() ){ assert(false); }
			return final_result.is_succeed() ? result : parse_results::expected_failed;
		}

		final_result = parse_results::worse(final_result, result);
	}

	attr = ret;
	return final_result;
}

shared_ptr<parser> queuer::clone() const
{
	return make_shared<queuer>( *this );
}

negnativer::negnativer( boost::shared_ptr<parser> p ): expr(p){}
negnativer::negnativer( negnativer const& rhs ): expr(rhs.expr){}

parse_results negnativer::parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr, diag_chat* diags ) const{
	if ( !expr ) {return parse_results::failed;}
	return ( expr->parse(iter, end, attr, diags).is_succeed() ) ? parse_results::failed : parse_results::succeed;
}

boost::shared_ptr<parser> negnativer::clone() const{
	return make_shared<negnativer>( *this );
}

rule::rule() : preset_id(-1){}

rule::rule( intptr_t id ) : preset_id(id){}

rule::rule( rule const& rhs ) : expr(rhs.expr), preset_id(rhs.preset_id){}

rule::rule( shared_ptr<parser> expr, intptr_t id /*= -1 */ ) : expr(expr), preset_id(id){}

rule::rule( parser const& rhs ) : expr(rhs.clone()){}

intptr_t rule::id() const{
	if( preset_id >= 0 ) return preset_id;
	return (intptr_t)this;
}

std::string const& rule::name() const{
	return rule_name;
}

void rule::name( std::string const & v ){
	rule_name = v;
}

struct path_tree
{
	std::string									element;
	vector< std::pair<string, boost::format> >	attributes;
	vector< shared_ptr<path_tree> >				children;
};

vector< shared_ptr<path_tree> > path_stack;

void print_path_tree( std::ostream& o, shared_ptr<path_tree> const& path_node, int indent = 0 )
{
	/*
	// Print Element
	for( int i = 0; i < indent; ++i ){ o << "  "; }
	o << "<" << path_node->element;

	// Print Attribute
	for( size_t i = 0; i < path_node->attributes.size(); ++i )
	{
		o << " " << path_node->attributes[i].first << "=" << path_node->attributes[i].second;
	}

	if( path_node->children.empty() )
	{
		o << " />" << endl;
	}
	else
	{
		o << " >" << endl;
		for( size_t i = 0; i < path_node->children.size(); ++i )
		{
			print_path_tree( o, path_node->children[i], indent+1 );
		}
		for( int i = 0; i < indent; ++i ){ o << "  "; }
		o << "</" << path_node->element << ">" << endl;
	}
	*/
}

std::fstream* pf = NULL;

#define SASL_PARSER_LOG_ENABLED 0
parse_results rule::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr, diag_chat* diags ) const{
	static int indent = 0;
	
	if(!pf){
		pf = new std::fstream( "rules.xml", std::ios::out );
	}

	shared_ptr<diag_chat> chat = diag_chat::create();
	
	assert( expr );
	if( !expr ){ return parse_results::failed; }

	if( rule_name == "assignexpr" )
	{
		// cout << "fuck" << endl;
	}

	shared_ptr<path_tree> current_path( make_shared<path_tree>() );
	if( !path_stack.empty() )
	{
		path_stack.back()->children.push_back( current_path );
	}
	
	path_stack.push_back( current_path );
	current_path->element = rule_name;
	parse_results result = expr->parse(iter, end, attr, chat.get());
	current_path->attributes.push_back( make_pair( "diag_count", boost::format("\"%d\"") % chat->diag_items().size() ) );
	if( result.is_succeed() ) {
		current_path->attributes.push_back( make_pair( "state", boost::format("\"%s\"") % "succeed" ) );
	} else if ( result.is_recovered() ) {
		current_path->attributes.push_back( make_pair( "state", boost::format("\"%s\"") % "recovered" ) );
	} else if ( result.is_failed() ) {
		current_path->attributes.push_back( make_pair( "state", boost::format("\"%s\"") % "failed" ) );
	} else if ( result.is_expected_failed() ) {
		current_path->attributes.push_back( make_pair( "state", boost::format("\"%s\"") % "expected_failed" ) );
	} else if ( result.is_recovered_expected_failed() ) {
		current_path->attributes.push_back( make_pair( "state", boost::format("\"%s\"") % "recovered_expected_failed" ) );
	}

	current_path->attributes.push_back( make_pair("position", boost::format("\"%d, %d\"") % (*iter)->span.line_beg % (*iter)->span.col_beg) );
	current_path->attributes.push_back( make_pair("token_content", boost::format("\"%s\"") % (*iter)->str) );

	path_stack.pop_back();

	if( path_stack.empty() )
	{
		print_path_tree( *pf,current_path );
		pf->close();
		delete pf;
		pf = NULL;
	}

	if( result.is_continuable() ){
		attr->rule_id( id() );
	}
	if( !result.is_succeed() ){
		diag_chat::merge( diags, chat.get(), true );
	}

	return result;
}

shared_ptr<parser> rule::clone() const{
	return make_shared<rule_wrapper>(*this);
}

rule& rule::operator=( parser const& rhs ){
	expr = rhs.clone();
	return *this;
}

rule& rule::operator=( rule const& rhs )
{
	expr = rhs.clone();
	return *this;
}

parser const* rule::get_parser() const
{
	return expr.get();
}

rule_wrapper::rule_wrapper( rule_wrapper const& rhs ) : r(rhs.r){}

rule_wrapper::rule_wrapper( rule const & rhs ) : r(rhs){}

parse_results rule_wrapper::parse(
	token_iterator& iter, token_iterator end,
	shared_ptr<attribute>& attr, diag_chat* diags ) const
{
	return r.parse(iter, end, attr, diags);
}

shared_ptr<parser> rule_wrapper::clone() const{
	return make_shared<rule_wrapper>(*this);
}

std::string const& rule_wrapper::name() const{
	return r.name();
}

rule const* rule_wrapper::get_rule() const
{
	return &r;
}

endholder::endholder(){}

endholder::endholder( endholder const & ){}
parse_results endholder::parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr, diag_chat* /*diags*/ ) const{
	if( iter == end ){
		attr = make_shared<terminal_attribute>();
		return parse_results::succeed;
	}
	return parse_results::failed;
}
boost::shared_ptr<parser> endholder::clone() const{
	return make_shared<endholder>();
}

repeater operator * ( parser const & expr ){
	return repeater( 0, repeater::unlimited, expr.clone() );
}

repeater operator - ( parser const& expr ){
	return repeater( 0, 1, expr.clone() );
}

selector operator | ( parser const & expr0, parser const& expr1 ){
	return selector()
		.add_branch( expr0.clone() )
		.add_branch( expr1.clone() );
}

selector operator | ( selector const & expr0, parser const& expr1 ){
	selector ret(expr0);
	return ret.add_branch( expr1.clone() );
}

selector operator | ( selector const & expr0, selector const & expr1 ){
	selector ret(expr0);
	BOOST_FOREACH( shared_ptr<parser> expr, expr1.branches() ){
		ret.add_branch(expr);
	}
	return ret;
}

queuer operator >> ( parser const& expr0, parser const& expr1 ){
	return queuer().append(expr0.clone()).append(expr1.clone());
}

queuer operator >> ( queuer const& expr0, parser const& expr1 ){
	return queuer(expr0).append(expr1.clone());
}

queuer operator > ( parser const& expr0, parser const& expr1 ){
	return queuer().append(expr0.clone()).append(expr1.clone(), true);
}

queuer operator > ( queuer const& expr0, parser const& expr1 ){
	return queuer(expr0).append(expr1.clone(), true);
}

negnativer operator!( parser const& expr1 ){
	return negnativer( expr1.clone() );
}

error_catcher::error_catcher( shared_ptr<parser> const& p, error_handler err_handler )
	: expr(p), err_handler(err_handler)
{
}

error_catcher::error_catcher( error_catcher const& rhs )
	: expr(rhs.expr), err_handler( rhs.err_handler )
{
}

shared_ptr<parser> error_catcher::clone() const
{
	return make_shared<error_catcher>( *this );
}

parse_results error_catcher::parse( token_iterator& iter, token_iterator end, boost::shared_ptr<attribute>& attr, sasl::common::diag_chat* diags ) const
{
	token_iterator origin_iter = iter;

	shared_ptr<diag_chat> children_diags = make_shared<diag_chat>();
	parse_results result = expr->parse(iter, end, attr, children_diags.get() );
	if( !result.is_continuable() )
	{
		result = err_handler( children_diags.get(), origin_iter, iter );
	}

	if( !result.is_succeed() )
	{
		diag_chat::merge( diags, children_diags.get(), true );
	}

	return result;
}

END_NS_SASL_PARSER();