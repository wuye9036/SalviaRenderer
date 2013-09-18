#include <sasl/include/common/diag_item.h>

using eflib::fixed_string;

BEGIN_NS_SASL_COMMON();

diag_item::diag_item( diag_template const* tmpl )
	: tmpl(tmpl)
{
}

bool diag_item::is_template( diag_template const& v ) const
{
	return tmpl == &v;
}

diag_item& diag_item::span( token_t const& beg, token_t const& end )
{
	item_span = code_span::merge( beg.span, end.span );
	return *this;
}

diag_item& diag_item::span( code_span const& s )
{
	item_span = s;
	return *this;
}

code_span diag_item::span() const
{
	return item_span;
}

diag_item& diag_item::file( fixed_string const& f )
{
	item_file = f;
	return *this;
}

fixed_string diag_item::file() const
{
	return item_file.empty() ? fixed_string("<unknown>") : item_file;
}

void diag_item::release()
{
	delete this;
}

diag_item& diag_item::eval()
{
	fmt.reset( new boost::format( this->str().raw_string() ) );
	return *this;
}

diag_levels diag_item::level() const
{
	return tmpl->level();
}

fixed_string diag_item::str() const
{
	for(size_t i = 0; i < fmt_params.size(); ++i)
	{
		fmt_params[i]->apply( const_cast<diag_item*>(this)->formatter() );
		fmt_params[i]->release();
	}
	fmt_params.clear();
	return fixed_string( formatter().str() );
}

size_t diag_item::id() const
{
	return tmpl->id();
}

boost::format& diag_item::formatter()
{
	if(!fmt)
	{
		fmt.reset(new boost::format( tmpl->template_str() ) );
	}
	return *fmt;
}

boost::format const& diag_item::formatter() const
{
	if(!fmt)
	{
		const_cast<diag_item*>(this)->fmt.reset(
			new boost::format( tmpl->template_str() )
			);
	}
	return *fmt;
}

diag_item::~diag_item()
{
	for(size_t i = 0; i < fmt_params.size(); ++i)
	{
		fmt_params[i]->release();
	}
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
	: uid(uid), lvl(lvl), tmpl(str)
{
}

diag_template::diag_template( diag_levels lvl, std::string const& str )
	: uid(automatic_id()), lvl(lvl), tmpl(str)
{
}

diag_levels diag_template::level() const
{
	return lvl;
}

size_t diag_template::id() const
{
	return uid;
}

END_NS_SASL_COMMON();

