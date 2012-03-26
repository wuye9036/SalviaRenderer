#include <sasl/include/driver/code_sources.h>

#include <sasl/include/common/diag_chat.h>
#include <sasl/include/parser/diags.h>

#include <eflib/include/diagnostics/assert.h>

using sasl::common::diag_chat;
using sasl::common::code_span;
using sasl::common::diag_item_committer;
using boost::shared_ptr;
using boost::wave::preprocess_exception;
using std::string;
using std::cout;
using std::endl;

BEGIN_NS_SASL_DRIVER();

bool driver_code_source::set_code( string const& code )
{
	this->code = code;
	this->filename = "<In Memory>";
	return process();
}

bool driver_code_source::set_file( string const& file_name )
{
	std::ifstream in(file_name.c_str(), std::ios_base::in);
	if (!in){
		return false;
	} else {
		in.unsetf(std::ios::skipws);
		std::copy(
			std::istream_iterator<char>(in), std::istream_iterator<char>(),
			std::back_inserter(code) );
		filename = file_name;
	}
	in.close();

	return process();
}

bool driver_code_source::eof()
{
	return next_it == wctxt->end();
}

string driver_code_source::next()
{
	assert( next_it != wctxt->end() );
	cur_it = next_it;

	try{
		++next_it;
	} catch ( preprocess_exception& e ){
		shared_ptr<diag_item_committer> committer;
		switch( e.get_errorcode() )
		{
		case preprocess_exception::no_error:
			break;
		case preprocess_exception::last_line_not_terminated:
			committer = diags->report( sasl::parser::boost_wave_exception_warning );
			break;
		default:
			EFLIB_ASSERT_UNIMPLEMENTED();
			break;
		}

		if( committer )
		{
			committer
				->p( preprocess_exception::error_text( e.get_errorcode() ) )
				->span( current_span() )
				->file( to_std_string(cur_it->get_position().get_file()) );
		}
		errtok = to_std_string( cur_it->get_value() );
		next_it = wctxt->end();
	}

	return to_std_string( cur_it->get_value() ) ;
}

string driver_code_source::error()
{
	if( errtok.empty() ){
		errtok = to_std_string( cur_it->get_value() );
	}
	return errtok;
}

const std::string& driver_code_source::file_name() const
{
	assert( cur_it != wctxt->end() );

	filename = to_std_string( cur_it->get_position().get_file() );
	return filename;
}

size_t driver_code_source::column() const
{
	assert( cur_it != wctxt->end() );
	return cur_it->get_position().get_column();
}

size_t driver_code_source::line() const
{
	assert( cur_it != wctxt->end() );
	return cur_it->get_position().get_line();
}

void driver_code_source::update_position( const std::string& /*lit*/ )
{
	// Do nothing.
	return;
}

bool driver_code_source::process()
{
	wctxt.reset( new wcontext_t( code.begin(), code.end(), filename.c_str() ) );

	size_t lang_flag = wctxt->get_language();
	lang_flag &= ~(boost::wave::support_option_emit_line_directives );
	lang_flag &= ~(boost::wave::support_option_single_line );
	lang_flag &= ~(boost::wave::support_option_emit_pragma_directives );
	wctxt->set_language( static_cast<boost::wave::language_support>( lang_flag ) );

	cur_it = wctxt->begin();
	next_it = wctxt->begin();

	return true;
}

void driver_code_source::set_diag_chat( sasl::common::diag_chat* diags )
{
	this->diags = diags;
}

driver_code_source::driver_code_source()
	: diags(NULL)
{

}

code_span driver_code_source::current_span() const
{
	return code_span( cur_it->get_position().get_line(), cur_it->get_position().get_column(), 1 );
}

END_NS_SASL_DRIVER();