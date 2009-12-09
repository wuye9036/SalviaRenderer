#include "include/vm/vm.h"
#include <tchar.h>
#include <iostream>
#include <string>
#include <vector>


int _tmain(int argc, _TCHAR* argv[])
{
	code_generator cg;
	
	cg
		//读取两个常量10，67
		.op( op_loadrc, r0, 10 )
		.op( op_loadrc, r1, 67 )
		//保存寄存器
		.op( op_push, r0 )
		.op( op_push, r1 )
		//将参数压栈
		.op( op_push, r0 )
		.op( op_push, r1 )
		// 调用call，目标为halt后面的子函数
		.op( op_call, 0x9 ) //跳到op_halt后面的指令
		.op( op_nop )		//这里一般处理返回值，不过因为直接是放在r0里面的，就返回好了
		// 终止运行。这里通常可以将寄存器弹栈，以返回保护的现场。
		.op( op_halt )
		// 子函数
		//将栈上的参数传递到寄存器中。
		.op( op_loadrs, r0, -sizeof(int)*2 - sizeof(int)*2 ) //注意，这里要跳过压到栈上的ebp和eip
		.op( op_loadrs, r1, -sizeof(int)*2 - sizeof(int) )
		.op( op_add, r0, r1) //执行加法
		.op( op_ret ) // 执行结果直接在r0中返回
		;

	vm machine;
	int result = machine.raw_call( cg.codes() );

	std::cout << result << endl;

	system("pause");
	return 0;
}

