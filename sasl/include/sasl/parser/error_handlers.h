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

typedef std::shared_ptr< sasl::common::token_t > token_ptr;
typedef std::vector< token_ptr > token_seq;
typedef token_seq::iterator token_iterator;

error_handler get_expected_failed_handler( std::string const& expected_str );
parse_results expected_failed_handler	( sasl::common::diag_chat* diags, token_iterator const& org_iter, token_iterator& iter, std::string const& expected_str );

}
