macro( config_llvm_libs )
set( SASL_LLVM_LIBS
		EFLIB sasl_codegen
		LLVMSupport
		LLVMJIT
		LLVMInterpreter
		LLVMX86CodeGen
		LLVMBitWriter
		LLVMBitReader
		LLVMAsmParser
		LLVMRuntimeDyld
		LLVMExecutionEngine
		LLVMAsmPrinter
		LLVMSelectionDAG
		LLVMX86Desc
		LLVMMCParser
		LLVMCodeGen
		LLVMX86AsmPrinter
		LLVMX86Info
		LLVMScalarOpts
		LLVMX86Utils
		LLVMInstCombine
		LLVMTransformUtils
		LLVMipa
		LLVMAnalysis
		LLVMTarget
		LLVMMC
		LLVMObject
		LLVMCore
	)
endmacro()

config_llvm_libs()