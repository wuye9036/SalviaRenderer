#ifndef SASL_COMMON_DIAG_CHAT_H
#define SASL_COMMON_DIAG_CHAT_H

#include <sasl/include/common/common_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SASL_COMMON();

struct	token_t;
class	diag_chat;
class	diag_item;
class	diag_template;

typedef boost::function<bool (diag_chat*, diag_item*)> report_handler_fn;

class diag_chat
{
public:
	static boost::shared_ptr<diag_chat> create();
	static diag_chat* merge( diag_chat* dest, diag_chat* src );
	void add_report_raised_handler( report_handler_fn const& handler );
	diag_item& report( token_t& beg, token_t& end, diag_template const& tmpl );
	diag_item& report( diag_template const& tmpl );
	~diag_chat();
private:
	std::vector<report_handler_fn>	handlers;
	std::vector<diag_item*>			diags;
};

END_NS_SASL_COMMON();

#endif