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
	diag_item* ret = new diag_item(&tmpl);
	diags.push_back( ret );
	ret->span( beg, end );
	for( vector<report_handler_fn>::iterator it = handlers.begin(); it != handlers.end(); ++it )
	{
		(*it)( this, ret );
	}
	return *ret;
}

END_NS_SASL_COMMON();