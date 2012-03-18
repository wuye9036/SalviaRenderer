#ifndef SASL_DRIVER_CODE_SOURCES_H
#define SASL_DRIVER_CODE_SOURCES_H

#include <sasl/include/driver/driver_forward.h>

#include <sasl/include/common/lex_context.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/scoped_ptr.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl
{
	namespace common
	{
		class diag_chat;
	}
}

BEGIN_NS_SASL_DRIVER();

class driver_code_source: public sasl::common::lex_context, public sasl::common::code_source{

private:
	typedef boost::wave::cpplexer::lex_iterator<
		boost::wave::cpplexer::lex_token<> >
		wlex_iterator_t;
	typedef boost::wave::context<
		std::string::iterator, wlex_iterator_t>
		wcontext_t;

public:
	void set_diag_chat( sasl::common::diag_chat* );
	bool set_code( std::string const& );
	bool set_file( std::string const& );

	// code source
	virtual bool		eof();
	virtual std::string next();
	virtual std::string error();

	// lex_context
	virtual std::string const&	file_name() const;
	virtual size_t				column() const;
	virtual size_t				line() const;
	virtual void				update_position( std::string const& /*lit*/ );

private:
	bool process();
	template<typename StringT>
	std::string to_std_string( StringT const& str ) const
	{
		return std::string( str.begin(), str.end() );
	}

	boost::scoped_ptr<wcontext_t>	wctxt;
	sasl::common::diag_chat*		diags;

	std::string			code;
	std::string			errtok;
	mutable std::string	filename;

	wcontext_t::iterator_type cur_it;
	wcontext_t::iterator_type next_it;
};

END_NS_SASL_DRIVER();

#endif