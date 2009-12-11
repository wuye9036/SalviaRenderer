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
	binary_expression bin_expr;

	std::string str("3+2");
	char const* first = str.c_str();
	char const* last = &first[str.size()];

	sasl_tokenizer sasl_tok;
	binary_expression_grammar<sasl_token_iterator> g( sasl_tok );

	try{
		bool r = boost::spirit::lex::tokenize_and_parse( first, last, sasl_tok, g, bin_expr );
		if (r){
			cout << "ok" << endl;
		} else {
			cout << "fail" << endl;
		}
	} catch (const std::runtime_error& e){
		cout << e.what() << endl;
	}

	//bin_expr.op = operators::add;
	//bin_expr.left_expr.reset( new constant( 10 ) );
	//bin_expr.right_expr.reset( new constant( 67 ) );

	vm_codegen vm_cg;
	vm_cg
		.emit_expression( bin_expr );
	//code_generator cg;

	//cg
	//	//读取两个常量10，67
	//	.op( op_loadrc, r0, 10 )
	//	.op( op_loadrc, r1, 67 )
	//	//保存寄存器
	//	.op( op_push, r0 )
	//	.op( op_push, r1 )
	//	//将参数压栈
	//	.op( op_push, r0 )
	//	.op( op_push, r1 )
	//	// 调用call，目标为halt后面的子函数
	//	.op( op_call, 0x9 ) //跳到op_halt后面的指令
	//	.op( op_nop )		//这里一般处理返回值，不过因为直接是放在r0里面的，就返回好了
	//	// 终止运行。这里通常可以将寄存器弹栈，以返回保护的现场。
	//	.op( op_halt )
	//	// 子函数
	//	//将栈上的参数传递到寄存器中。
	//	.op( op_loadrs, r0, -sizeof(int)*2 - sizeof(int)*2 ) //注意，这里要跳过压到栈上的ebp和eip
	//	.op( op_loadrs, r1, -sizeof(int)*2 - sizeof(int) )
	//	.op( op_add, r0, r1) //执行加法
	//	.op( op_ret ) // 执行结果直接在r0中返回
	//	;

	vm machine;
	int result = machine.raw_call( vm_cg.codes() );

	std::cout << result << endl;

	system("pause");
	return 0;
}

