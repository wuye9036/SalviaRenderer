#include <sasl/include/common/diag_item.h>

BEGIN_NS_SASL_COMMON();

diag_item::diag_item( diag_template const* tmpl )
	: tmpl(tmpl), fmt( tmpl->template_str() )
{
}

bool diag_item::is_template( diag_template const& v )
{
	return tmpl == &v;
}

diag_item& diag_item::span( token_t const& beg, token_t const& end )
{
	item_span = code_span::merge( beg.span, end.span );
	return *this;
}


std::string const& diag_template::template_str() const
{
	return tmpl;
}

size_t diag_template::automatic_id()
{
	static size_t id = 100;
	return ++id;
}

diag_template::diag_template( size_t uid, diag_levels lvl, std::string const& str )
	: uid(uid), level(lvl), tmpl(str)
{
}

diag_template::diag_template( diag_levels lvl, std::string const& str )
	: uid(automatic_id()), level(lvl), tmpl(str)
{
}

END_NS_SASL_COMMON();

