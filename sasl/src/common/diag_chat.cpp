#include <sasl/include/common/diag_chat.h>

#include <sasl/include/common/diag_item.h>
#include <eflib/include/diagnostics/assert.h>

using boost::shared_ptr;
using std::vector;

BEGIN_NS_SASL_COMMON();

shared_ptr<diag_chat> diag_chat::create()
{
	return shared_ptr<diag_chat>( new diag_chat() );
}

void diag_chat::add_report_raised_handler( report_handler_fn const& handler )
{
	handlers.push_back( handler );
}

diag_item& diag_chat::report( token_t& beg, token_t& end, diag_template const& tmpl )
{
	return report(tmpl).span(beg, end);
}

diag_item& diag_chat::report( diag_template const& tmpl )
{
	diag_item* ret = new diag_item(&tmpl);
	diags.push_back( ret );
	for( vector<report_handler_fn>::iterator it = handlers.begin(); it != handlers.end(); ++it )
	{
		(*it)( this, ret );
	}
	return *ret;
}

diag_chat* diag_chat::merge( diag_chat* dest, diag_chat* src )
{
	dest->diags.insert( dest->diags.end(), src->diags.begin(), src->diags.end() );
	src->diags.clear();
	return dest; 
}

diag_chat::~diag_chat()
{
	for( vector<diag_item*>::iterator it = diags.begin(); it != diags.end(); ++it )
	{
		(*it)->release();
	}
}

END_NS_SASL_COMMON();