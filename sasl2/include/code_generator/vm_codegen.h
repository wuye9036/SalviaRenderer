#ifndef SASL_CODE_GENERATOR_VM_CODEGEN_H
#define SASL_CODE_GENERATOR_VM_CODEGEN_H

#include "../vm/vm.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/constant.h"

class vm_codegen{
	vector<instruction> ins_;

public:
	vm_codegen& emit_expression( const binary_expression& expr ){
		if ( expr.op != operators::add ){
			return *this;
		}

		int c0 = expr.left_expr->val;
		int c1 = expr.right_expr->val;
		ins_.push_back( instruction( op_loadrc, r0, c0 ) );
		ins_.push_back( instruction( op_loadrc, r1, c1 ) );
		ins_.push_back( instruction( op_add, r0, r1 ) );
		return *this;
	}

	vm_codegen& emit_op( op_code op, int arg0 = 0, int arg1 = 0 ){
		ins_.push_back( instruction(op, arg0, arg1) );
		return *this;
	}
	const vector<instruction>& codes(){
		return ins_;
	}


};

#endif //SASL_CODE_GENERATOR_VM_CODEGEN_H