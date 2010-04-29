#include <tchar.h>
#include <vector>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/PassManager.h>
#include <llvm/CallingConv.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

LLVMContext ctxt;

Module* makeLLVMModule() {
	// Module Construction
	Module* mod = new Module("test", ctxt);
	Constant* c = mod->getOrInsertFunction("mul_add",
		/*ret type*/                           IntegerType::get(ctxt, 32),
		/*args*/                               IntegerType::get(ctxt, 32),
		IntegerType::get(ctxt, 32),
		IntegerType::get(ctxt, 32),
		/*varargs terminated with null*/       NULL);

	Function* mul_add = cast<Function>(c);
	mul_add->setCallingConv(CallingConv::C);
	Function::arg_iterator args = mul_add->arg_begin();
	Value* x = args++;
	x->setName("x");
	Value* y = args++;
	y->setName("y");
	Value* z = args++;
	z->setName("z");
	BasicBlock* block = BasicBlock::Create(ctxt, "entry", mul_add);
	IRBuilder<> builder(block);
	Value* tmp = builder.CreateBinOp(Instruction::Mul,
		x, y, "tmp");
	Value* tmp2 = builder.CreateBinOp(Instruction::Add,
		tmp, z, "tmp2");

	builder.CreateRet(tmp2);

	return mod;
}


int _tmain( int argc, TCHAR** argv){
	Module* Mod = makeLLVMModule();

	verifyModule(*Mod, PrintMessageAction);

	PassManager PM;
	PM.add(createPrintModulePass(&outs()));
	PM.run(*Mod);

	delete Mod;
	system("pause");
	return 0;
}