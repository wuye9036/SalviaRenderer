#include <sasl/include/parser/parse_api.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/grammars.h>
#include <sasl/include/parser_tree/program.h>

#include <iostream>

using std::cout;
using std::endl;

using boost::spirit::lex::tokenize;
using boost::spirit::lex::tokenize_and_phrase_parse;

template <typename ParserTreeT>
void sasl::parser::detail::parser<ParserTreeT>::parse(
	ParserTreeT& pt_root,
	const std::string& code,
	boost::shared_ptr< ::sasl::common::lex_context > ctxt )
{
	const char* lex_first = &code[0];
	const char* lex_last = &code[0] + code.size();
	
	sasl_tokenizer tok( ctxt );
	sasl_grammar<sasl_token_iterator, sasl_skipper, sasl_tokenizer> sasl_parser(tok);

	try{
		try{
			// Try to use all lex state for tokenize character sequence.

			const size_t tok_states_count = 2;
			const char* tok_states[tok_states_count] = {NULL, "SKIPPED"};

			int toked_state = 0; // 0 is no result, 1 is succeed, 2 is failed.
			int i_state = 0;
			while( lex_first != lex_last && toked_state == 0 ){

				const char* next_lex_first = lex_first;

				tokenize( next_lex_first, lex_last, tok, tok_states[i_state] );
				
				// next state.
				i_state = (++i_state) % tok_states_count;

				if( next_lex_first == lex_last ){
					toked_state = 1;
					break;
				}

				if( next_lex_first == lex_first ){
					toked_state = 2;
					break;
				}

				lex_first = next_lex_first;
			}
			
			bool tokenize_succeed = (toked_state == 1);
			if( tokenize_succeed ){
				// do nothing
			} else {
				throw std::runtime_error( "tokenization failed!" );
			}

			const char* parse_first = &code[0];
			const char* parse_last = &code[0] + code.size();
			
			bool parse_succeed = tokenize_and_phrase_parse(
				parse_first, parse_last, tok, sasl_parser.prog(), SASL_PARSER_SKIPPER( tok ), pt_root);
			if (parse_succeed){
				cout << "Parser finished." << endl;
			} else {
				cout << "Parser failed." << endl;
			}
		} catch ( boost::spirit::qi::expectation_failure<sasl_token_iterator> const& x) {
			std::cout << "expected: " << x.what_;
		}
	} catch (const std::runtime_error& e){
		cout << e.what() << endl;
	}
}

void instantiate(){
	std::string s;
	sasl::parser_tree::program prog;
	boost::shared_ptr< ::sasl::common::lex_context > ctxt;
	sasl::parser::parse( prog, s, ctxt );
}