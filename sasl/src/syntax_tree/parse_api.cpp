#include <sasl/include/syntax_tree/parse_api.h>

#include <sasl/include/parser/parse_api.h>
#include <sasl/include/parser_tree/program.h>
#include <sasl/include/syntax_tree/syntax_tree_builder.h>

using sasl::common::lex_context;
using boost::shared_ptr;

BEGIN_NS_SASL_SYNTAX_TREE();

shared_ptr<program> pt_to_ast(
	const sasl::parser_tree::program& pt_prog
	)
{
	syntax_tree_builder builder;
	return builder.build( pt_prog );
}

boost::shared_ptr<program> parse(
	const std::string& code_text,
	boost::shared_ptr<lex_context> ctxt
	)
{
	sasl::parser_tree::program pt_prog;
	sasl::parser::parse( pt_prog, code_text, ctxt );
	return pt_to_ast( pt_prog );
}

END_NS_SASL_SYNTAX_TREE();

