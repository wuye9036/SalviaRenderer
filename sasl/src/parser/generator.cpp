#include <sasl/include/parser/generator.h>

#include <sasl/include/common/token.h>
#include <sasl/include/parser/lexer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/preprocessor.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::shared_ptr;
using boost::make_shared;
using std::vector;

BEGIN_NS_SASL_PARSER();

//////////////////////////////////////////////////////////////////////////
// Attributes.
attribute::attribute() :rid(-1){}
attribute::~attribute(){}
intptr_t attribute::rule_id() const{ return rid; }
void attribute::rule_id( intptr_t id ){ rid = id; }

void terminal_attribute::accept( attribute_visitor& v, boost::any& ctxt ){}

void sequence_attribute::accept( attribute_visitor& v, boost::any& ctxt ){}

selector_attribute::selector_attribute() : selected_idx(-1){}
void selector_attribute::accept( attribute_visitor& v, boost::any& ctxt ){}

void queuer_attribute::accept( attribute_visitor& v, boost::any& ctxt ){}

//////////////////////////////////////////////////////////////////////////
// Parsers


terminal::terminal( size_t tok_id ) :tok_id(tok_id){}

terminal::terminal( terminal const& rhs ) :tok_id(rhs.tok_id){}

bool terminal::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr ) const
{
	if ( iter == end ){
		return false;
	}

	if( (*iter)->id == tok_id ){
		shared_ptr<terminal_attribute> ret = make_shared<terminal_attribute>();
		ret->tok = *iter;
		attr = ret;

		++iter;
		return true;
	}

	return false;
}

shared_ptr<parser> terminal::clone() const
{
	return make_shared<terminal>(*this);
}

size_t const repeater::unlimited = std::numeric_limits<size_t>::max();

repeater::repeater( size_t lower_bound, size_t upper_bound, shared_ptr<parser> expr ) : lower_bound(lower_bound), upper_bound(upper_bound), expr(expr){}

repeater::repeater( repeater const& rhs ) : lower_bound(rhs.lower_bound), upper_bound(rhs.upper_bound), expr(rhs.expr){}

bool repeater::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr ) const
{
	token_iterator stored = iter;
	size_t matched_count = 0;

	shared_ptr<sequence_attribute> seq_attr = make_shared<sequence_attribute>();
	shared_ptr<attribute> out;
	while( expr->parse(iter, end, out) ){
		seq_attr->attrs.push_back(out);
		++matched_count;
		if( matched_count == upper_bound ){
			break;
		}
	}
	if( matched_count < lower_bound ){
		iter = stored;
		return false;
	}

	attr = seq_attr;
	return true;
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

bool selector::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr ) const
{
	shared_ptr<selector_attribute> slc_attr = make_shared<selector_attribute>();

	int idx = 0;
	BOOST_FOREACH( shared_ptr<parser> p, branches() )
	{
		if( p->parse(iter, end, attr) ){
			slc_attr->attr = attr;
			slc_attr->selected_idx = idx;
			return true;
		}
		++idx;
	}

	return false;
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

queuer& queuer::append( shared_ptr<parser> p )
{
	exprlst.push_back(p);
	return *this;
}

std::vector< shared_ptr<parser> > const& queuer::exprs() const
{
	return exprlst;
}

bool queuer::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr ) const
{
	token_iterator stored = iter;

	shared_ptr<attribute> out;
	shared_ptr<queuer_attribute> ret = make_shared<queuer_attribute>();

	BOOST_FOREACH( shared_ptr<parser> p, exprlst ){
		if( ! p->parse(iter, end, out ) ){
			iter = stored;
			return false;
		}
		ret->attrs.push_back(out);
	}

	attr = ret;
	return true;
}

shared_ptr<parser> queuer::clone() const
{
	return make_shared<queuer>( *this );
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

bool rule::parse( token_iterator& iter, token_iterator end, shared_ptr<attribute>& attr ) const{
	if( !expr ){
		return false;
	}
	if( expr->parse(iter, end, attr) ){
		attr->rule_id( id() );
		return true;
	}
	return false;
}

shared_ptr<parser> rule::clone() const{
	return make_shared<rule_wrapper>(*this);
}

rule& rule::operator=( parser const& rhs ){
	if( &rhs == this ){
		return *this;
	}
	expr = rhs.clone();
	return *this;
}


rule_wrapper::rule_wrapper( rule_wrapper const& rhs ) : r(rhs.r){}

rule_wrapper::rule_wrapper( rule const & rhs ) : r(rhs){}

bool rule_wrapper::parse(
	token_iterator& iter, token_iterator end,
	shared_ptr<attribute>& attr ) const
{
	return r.parse(iter, end, attr);
}

shared_ptr<parser> rule_wrapper::clone() const{
	return make_shared<rule_wrapper>(*this);
}

repeater operator * ( parser const & expr ){
	return repeater( 0, repeater::unlimited, expr.clone() );
}

repeater operator ! ( parser const& expr ){
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

queuer operator >> ( queuer const& expr0, queuer const& expr1 ){
	queuer ret(expr0);
	BOOST_FOREACH( shared_ptr<parser> expr, expr1.exprs() ){
		ret.append(expr);
	}
	return ret;
}

END_NS_SASL_PARSER();