#pragma once

#include <sasl/parser/generator.h>

namespace sasl
{
	namespace common
	{
		class diag_chat;
	}
}

namespace sasl::parser{
typedef std::vector<token> token_seq;
typedef token_seq::iterator token_iterator;

error_handler get_expected_failed_handler( std::string const& expected_str );
parse_results expected_failed_handler	( sasl::common::diag_chat* diags, token_iterator const& org_iter, token_iterator& iter, std::string const& expected_str );

}
