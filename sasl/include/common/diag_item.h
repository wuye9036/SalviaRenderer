#ifndef SASL_COMMON_DIAG_ITEM_H
#define SASL_COMMON_DIAG_ITEM_H

#include <sasl/include/common/common_fwd.h>

#include <sasl/include/common/token.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_COMMON();

enum diag_levels{
	dl_fatal_error,
	dl_error,
	dl_warning,
	dl_info
};

class diag_template;

class diag_item
{
public:
	diag_item( diag_template const* tmpl );
	
	template <typename T>
	diag_item& operator % ( T const& v )
	{
		fmt % v;
		return *this;
	}

	diag_item& span( token_t const& beg, token_t const& end );
	bool is_template( diag_template const& v );

private:
	code_span				item_span;
	diag_template const*	tmpl;
	boost::format			fmt;
};

class diag_template
{
public:
	diag_template( size_t uid, diag_levels lvl, std::string const& str );
	diag_template( diag_levels lvl, std::string const& str );
	
	std::string const& template_str() const;
	static size_t automatic_id();
private:
	size_t			uid;
	diag_levels		level;
	std::string		tmpl;
};

END_NS_SASL_COMMON();

#endif