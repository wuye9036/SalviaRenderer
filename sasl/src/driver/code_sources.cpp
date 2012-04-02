#include <sasl/include/driver/code_sources.h>

#include <sasl/include/common/diag_chat.h>
#include <sasl/include/parser/diags.h>

#include <eflib/include/diagnostics/assert.h>

using sasl::common::diag_chat;
using sasl::common::code_span;
using sasl::common::diag_item_committer;
using boost::shared_ptr;
using boost::wave::preprocess_exception;
using boost::unordered_map;
using std::string;
using std::cout;
using std::endl;

BEGIN_NS_SASL_DRIVER();

boost::unordered_map<void*, driver_code_source*> driver_code_source::ctxt_to_source;

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
	return next_it == wctxt_wrapper->get_wctxt()->end();
}

string driver_code_source::next()
{
	assert( next_it != wctxt_wrapper->get_wctxt()->end() );
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
			is_failed = true;
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
		next_it = wctxt_wrapper->get_wctxt()->end();
	} catch( wave_reported_fatal_error& ) {
		// Error has been reported to diag_chat.
		errtok = to_std_string( cur_it->get_value() );
		next_it = wctxt_wrapper->get_wctxt()->end();
		is_failed = true;
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
	assert( cur_it != wctxt_wrapper->get_wctxt()->end() );

	filename = to_std_string( cur_it->get_position().get_file() );
	return filename;
}

size_t driver_code_source::column() const
{
	assert( cur_it != wctxt_wrapper->get_wctxt()->end() );
	return cur_it->get_position().get_column();
}

size_t driver_code_source::line() const
{
	assert( cur_it != wctxt_wrapper->get_wctxt()->end() );
	return cur_it->get_position().get_line();
}

void driver_code_source::update_position( const std::string& /*lit*/ )
{
	// Do nothing.
	return;
}

bool driver_code_source::process()
{
	wctxt_wrapper.reset( new wave_context_wrapper( this, new wcontext_t( code.begin(), code.end(), filename.c_str() ) ) );

	size_t lang_flag = wctxt_wrapper->get_wctxt()->get_language();
	lang_flag &= ~(boost::wave::support_option_emit_line_directives );
#if BOOST_VERSION <= 104900
	lang_flag |= (boost::wave::support_option_single_line );
#else
	lang_flag &= ~(boost::wave::support_option_single_line );
#endif
	lang_flag &= ~(boost::wave::support_option_emit_pragma_directives );
	wctxt_wrapper->get_wctxt()->set_language( static_cast<boost::wave::language_support>( lang_flag ) );

	cur_it = wctxt_wrapper->get_wctxt()->begin();
	next_it = wctxt_wrapper->get_wctxt()->begin();
	is_failed = false;

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

driver_code_source::~driver_code_source()
{
	
}

code_span driver_code_source::current_span() const
{
	return code_span( cur_it->get_position().get_line(), cur_it->get_position().get_column(), 1 );
}

void driver_code_source::add_virtual_file( std::string const& file_name, std::string const& content, bool high_priority )
{
	virtual_files[file_name] = make_pair( high_priority, content );
}

void driver_code_source::set_include_handler( include_handler_fn handler )
{
	inc_handler = handler;
}

driver_code_source* driver_code_source::get_code_source( void* ctxt )
{
	if( ctxt )
	{
		unordered_map<void*, driver_code_source*>::iterator it = ctxt_to_source.find(ctxt);
		if( it != ctxt_to_source.end() )
		{
			return it->second;
		}
	}

	return NULL;
}

bool driver_code_source::failed()
{
	return is_failed;
}

void report_load_file_failed( void* ctxt, std::string const& name, bool is_system )
{
	driver_code_source* code_src = driver_code_source::get_code_source(ctxt);
	code_src->diags
		->report( sasl::parser::cannot_open_include_file )
		->file(code_src->file_name() )->span( code_src->current_span() )
		->p(name);
}

void load_virtual_file(
	bool& is_succeed, bool& is_exclusive, std::string& content,
	void* ctxt, std::string const& name, bool is_system,
	bool before_file_load )
{
	is_exclusive = false;
	is_succeed = false;

	driver_code_source* code_src = driver_code_source::get_code_source(ctxt);
	assert( code_src );
	if( code_src->inc_handler )
	{
		std::string native_name;
		is_exclusive = true;
		is_succeed = code_src->inc_handler( content, native_name, name, is_system, false );
		return;
	}

	if( !is_system ) { return; }

	driver_code_source::virtual_file_dict::iterator it 
		= code_src->virtual_files.find(name);
	if ( it == code_src->virtual_files.end() )
	{
		return;
	}

	bool hi_prior = it->second.first;
	if( hi_prior == before_file_load )
	{
		content = it->second.second;
		is_succeed = true;
		return;
	}

	return;
}

void check_file( bool& is_succeed, bool& is_exclusive, string& native_name,
	void* ctxt, string const& name, bool is_system,
	bool is_before_include )
{
	is_exclusive = false;
	is_succeed = false;

	driver_code_source* code_src = driver_code_source::get_code_source(ctxt);
	assert( code_src );

	// If inc handler is enabled, it is exclusive. 
	if( code_src->inc_handler )
	{
		std::string content;
		is_exclusive = true;
		is_succeed = code_src->inc_handler( content, native_name, name, is_system, true );
		return;
	}

	driver_code_source::virtual_file_dict::iterator it 
		= code_src->virtual_files.find(name);
	if ( it == code_src->virtual_files.end() )
	{
		return;
	}

	bool hi_prior = it->second.first;
	if( hi_prior == is_before_include )
	{
		native_name = name;
		is_succeed = true;
		return;
	}

	return;
}

wave_context_wrapper::wave_context_wrapper( driver_code_source* src, wcontext_t* ctx )
{
	if( wctxt )
	{
		driver_code_source::ctxt_to_source.erase(wctxt.get());
	}
	driver_code_source::ctxt_to_source[ctx] = src;
	wctxt.reset(ctx);
}

wave_context_wrapper::~wave_context_wrapper()
{
	if( wctxt )
	{
		driver_code_source::ctxt_to_source.erase(wctxt.get());
	}
	wctxt.reset(NULL);
}

wcontext_t* wave_context_wrapper::get_wctxt() const
{
	return wctxt.get();
}

END_NS_SASL_DRIVER();