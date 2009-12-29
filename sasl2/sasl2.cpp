#include "enums/operators.h"
#include "include/code_generator/vm_codegen.h"
#include "include/syntax_tree/expression.h"
#include "include/syntax_tree/constant.h"
#include "include/parser/binary_expression.h"
#include "include/parser/token.h"

#include <tchar.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace std;
using namespace boost;

struct token_printer{
	template <typename TokenT>
	bool operator()( const TokenT& tok ){
		cout << "token " << get<token_attr>( tok.value() ).lit << " " << "at " << get<token_attr>( tok.value() ).column << endl;
		return true;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	binary_expression_ bin_expr_;
	binary_expression::handle_t bin_expr;

	std::string str("3+  2");
	char const* first = str.c_str();
	char const* last = &first[str.size()];

	sasl_tokenizer sasl_tok;
	binary_expression_grammar<sasl_token_iterator, sasl_skipper> g( sasl_tok );

	try{
		bool r = boost::spirit::lex::tokenize_and_phrase_parse( first, last, sasl_tok, g, SKIPPER( sasl_tok ), bin_expr_ );
		if (r){
			cout << "ok" << endl;
			bin_expr = syntax_tree_builder().build_expression( bin_expr_ );
			bin_expr->update();
		} else {
			cout << "fail" << endl;
		}
	} catch (const std::runtime_error& e){
		cout << e.what() << endl;
	}

	vm_codegen vm_cg;
	vm_cg
		.emit_expression( bin_expr );

	vm machine;
	intptr_t result = machine.raw_call( vm_cg.codes() );

	std::cout << result << endl;

	system("pause");
	return 0;
}

