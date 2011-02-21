#ifndef SASL_PARSER_PARSE_API_H
#define SASL_PARSER_PARSE_API_H

#include <sasl/include/parser/parser_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace common{
		class lex_context;
	}
	namespace parser{
		class attribute;
		void parse(
			boost::shared_ptr<attribute>& pt_root,
			const std::string& code,
			boost::shared_ptr< ::sasl::common::lex_context > ctxt
			);
	}
}

#endif