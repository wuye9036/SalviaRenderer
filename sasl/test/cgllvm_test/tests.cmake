set( SASL_CGLLVM_TEST_HEADERS "" )
set( SASL_CGLLVM_TEST_SOURCES "" )
set( SASL_CGLLVM_TEST_LIBS "" )

if( SOFTART_BUILD_WITH_LLVM )
	set( SASL_CGLLVM_TEST_HEADERS "" )
	set( SASL_CGLLVM_TEST_SOURCES
		${SASL_HOME_DIR}/sasl/test/cgllvm_test/program_test.cpp
		${SASL_HOME_DIR}/sasl/test/cgllvm_test/function_test_basic.cpp
	)
	set( SASL_CGLLVM_TEST_LIBS
		${SASL_LLVM_LIBS}
	)

	if ( WIN32 AND MINGW )
		set ( SASL_CGLLVM_TEST_LIBS ${SASL_CGLLVM_TEST_LIBS} imagehlp psapi )
	endif( WIN32 AND MINGW )

	ADD_DEFINITIONS(
		-DSASL_USE_LLVM
		-DSOFTART_USE_LLVM
	)
endif( SOFTART_BUILD_WITH_LLVM )
