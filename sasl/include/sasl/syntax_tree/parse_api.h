#ifndef SASL_SYNTAX_TREE_PARSE_API_H
#define SASL_SYNTAX_TREE_PARSE_API_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <memory>
#include <string>

namespace sasl{
	namespace common{
		class lex_context;
		class code_source;
		class diag_chat;
	}
}

namespace sasl::syntax_tree() {

struct program;

std::shared_ptr<program> parse(
	const std::string& code_text,
	std::shared_ptr<sasl::common::lex_context> ctxt,
	sasl::common::diag_chat* diags
	);

std::shared_ptr<program> parse(
	sasl::common::code_source* src,
	std::shared_ptr<sasl::common::lex_context> ctxt,
	sasl::common::diag_chat* diags
	);

}

#endif