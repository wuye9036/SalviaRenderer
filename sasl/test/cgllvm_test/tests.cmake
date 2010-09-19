set( SASL_CGLLVM_TEST_HEADERS "" ) 
set( SASL_CGLLVM_TEST_SOURCES "" )
set( SASL_CGLLVM_TEST_LIBS "" )

if( SOFTART_BUILD_WITH_LLVM )
	set( SASL_CGLLVM_TEST_HEADERS "" )
	set( SASL_CGLLVM_TEST_SOURCES 
		${sasl_home_dir}/sasl/test/cgllvm_test/program_test.cpp
		${sasl_home_dir}/sasl/test/cgllvm_test/function_test_basic.cpp
	)
	set( SASL_CGLLVM_TEST_LIBS 
		sasl_cgllvm LLVMCore LLVMSystem LLVMSupport
	)
endif( SOFTART_BUILD_WITH_LLVM )