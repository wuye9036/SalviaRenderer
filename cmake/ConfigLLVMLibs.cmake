macro( config_llvm_libs )
if (MSVC)
	set( SASL_LLVM_LIBS
		LLVMJIT LLVMInterpreter LLVMExecutionEngine
		LLVMX86CodeGen LLVMX86Desc LLVMX86Utils LLVMX86AsmPrinter LLVMX86Info
		LLVMBitWriter LLVMBitReader LLVMAsmParser LLVMAsmPrinter
		LLVMRuntimeDyld 
		LLVMSelectionDAG
		LLVMMCParser
		LLVMCodeGen 
		LLVMScalarOpts
		LLVMInstCombine
		LLVMTransformUtils
		LLVMipa LLVMAnalysis LLVMTarget LLVMMC
		LLVMObject LLVMCore LLVMSupport
	)
else(MSVC)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SALVIA_LLVM_INSTALL_DIR}/share/llvm/cmake")
	include(LLVMConfig)
	llvm_map_components_to_libraries(SASL_LLVM_LIBS jit native interpreter)
endif(MSVC)
endmacro()

config_llvm_libs()