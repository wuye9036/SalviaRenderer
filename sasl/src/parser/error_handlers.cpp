#include <sasl/include/parser/error_handlers.h>
#include <sasl/include/parser/diags.h>
#include <sasl/include/parser/generator.h>
#include <sasl/include/common/diag_chat.h>
#include <boost/bind.hpp>

using sasl::common::diag_chat;
using boost::bind;

BEGIN_NS_SASL_PARSER();

error_handler get_expected_failed_handler( std::string const& expected_str )
{
	return boost::bind( expected_failed_handler, _1, _2, _3, expected_str );
}

parse_results expected_failed_handler( diag_chat* diags, token_iterator const& org_iter, token_iterator& iter, std::string const& expected_str )
{
	token_ptr tok = *iter;
	
	diags->clear();
	diags->report( unmatched_expected_token )
		->span(tok->span)->file(tok->file_name)
		->p(expected_str)->p( tok->end_of_file ? "<eof>" : tok->str);

	iter = org_iter;

	return parse_results::recovered_expected_failed;
}

END_NS_SASL_PARSER();