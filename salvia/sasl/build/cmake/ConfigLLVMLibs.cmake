macro( config_llvm_libs )
set( SASL_LLVM_LIBS
		EFLIB sasl_cgllvm
		LLVMJIT
		LLVMInterpreter
		LLVMX86CodeGen
		LLVMExecutionEngine
		LLVMSelectionDAG
		LLVMX86AsmPrinter
		LLVMX86Info
		LLVMX86Utils
		LLVMCodeGen
		LLVMScalarOpts
		LLVMInstCombine
		LLVMTransformUtils
		LLVMipa
		LLVMAnalysis
		LLVMTarget
		LLVMCore
		LLVMMC
		LLVMSupport
		LLVMAsmPrinter
		LLVMMCParser
	)
endmacro()

config_llvm_libs()