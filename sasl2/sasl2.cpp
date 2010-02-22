//#include "enums/operators.h"
//#include "include/code_generator/vm_codegen.h"
//#include "include/syntax_tree/expression.h"
//#include "include/syntax_tree/constant.h"
//#include "include/syntax_tree/syntax_tree_builder.h"

#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_array.hpp>

#include "include/parser_tree/program.h"
#include "include/parser/grammars.h"

//using namespace sasl::vm;
//using namespace sasl::code_generator;
//
using namespace std;
using namespace boost;

struct token_printer{
	template <typename TokenT>
	bool operator()( const TokenT& tok ){
		cout << "token " << get<token_attr>( tok.value() ).lit << " " << "at " << get<token_attr>( tok.value() ).column << endl;
		return true;
	}
};

namespace pt = sasl::parser_tree;

int _tmain(int argc, _TCHAR* argv[])
{
	pt::program pt_prog;
	pt::declaration pt_decl;
	//expression::handle_t st_expr;

	fstream f("text.txt", ios_base::in);
	
	long file_size = f.seekg( -1, ios_base::end ).tellg();
	file_size += 1;
	shared_array<char> str( new char[file_size+1] );
	f.seekg(0, ios_base::beg).get(str.get(), file_size+1, 0);
	str[file_size] = 0;

	char const* first = &(str[0]);
	char const* last = &(str[file_size]);
	
	sasl_tokenizer sasl_tok;
	sasl_grammar<sasl_token_iterator, sasl_skipper, sasl_tokenizer> sasl_parser(sasl_tok);

	//try{
	//	try{
			//bool tok_r = tokenize( first, last, sasl_tok, token_printer() );
			//if(tok_r && (first == last) ){
			//	cout << "Tokenize Succeed!" << endl;
			//} else {
			//	throw runtime_error( "Tokenize Failed!" );
			//}

			//first = str.c_str();
			//last = &first[str.size()];
			bool r = boost::spirit::lex::tokenize_and_phrase_parse( first, last, sasl_tok, sasl_parser.prog(), SASL_PARSER_SKIPPER( sasl_tok ), pt_prog);
			if (r){
				cout << "ok" << endl;
				// st_expr = syntax_tree_builder_impl().build( pt_expr );
			} else {
				cout << "fail" << endl;
			}
	//	} 
	//	catch ( boost::spirit::qi::expectation_failure<sasl_token_iterator> const& x)
	//	{
	//		std::cout << "expected: " << x.what_;
	//		//std::cout << "got: \"" << std::string(x.first, x.last) << '"' << std::endl;
	//	}
	//} catch (const std::runtime_error& e){
	//	cout << e.what() << endl;
	//}

	////vm_codegen vm_cg;
	////vm_cg.emit_expression( bin_expr );

	////vm machine;
	////intptr_t result = machine.raw_call( vm_cg.codes() );

	////std::cout << result << endl;

	system("pause");
	return 0;
}

