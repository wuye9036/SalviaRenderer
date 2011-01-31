#ifndef SASL_SYNTAX_TREE_PARSE_API_H
#define SASL_SYNTAX_TREE_PARSE_API_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace common{
		class lex_context;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE()

struct program;

boost::shared_ptr<program> parse(
	const std::string& code_text,
	boost::shared_ptr< ::sasl::common::lex_context > ctxt
	);

END_NS_SASL_SYNTAX_TREE()

#endif