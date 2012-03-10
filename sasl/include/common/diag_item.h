
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
	diag_item( diag_template* tmpl ): tmpl(tmpl), fmt( tmpl->template_str() )
	{
	}
	
	template <typename T>
	diag_item& operator % ( T const& v )
	{
		fmt % v;
		return *this;
	}
	
	diag_item& span( token_t const& beg, token_t const& end );
	
	bool is_template( diag_template const& v )
	{
		return tmpl == &v;
	}
private:
	diag_template*	tmpl;
	boost::format	fmt;
};

class diag_template
{
public:
	diag_template( size_t uid, diag_levels lvl, std::string const& str );
	diag_template( diag_levels lvl, std::string const& str );
	
	std::string const& template_str();
	static size_t automatic_id();
private:
	size_t			uid;
	diag_levels		level;
	std::string		tmpl;
}