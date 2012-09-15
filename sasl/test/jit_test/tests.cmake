set( SASL_JIT_TEST_HEADERS "" )
set( SASL_JIT_TEST_SOURCES "" )
set( SASL_JIT_TEST_LIBS "" )

set( SASL_JIT_TEST_HEADERS
	${SASL_HOME_DIR}/sasl/test/jit_test/jit_test.h )
set( SASL_JIT_TEST_SOURCES
	${SASL_HOME_DIR}/sasl/test/jit_test/general.cpp
	${SASL_HOME_DIR}/sasl/test/jit_test/jit_test.cpp )
set( SASL_JIT_TEST_LIBS ${SASL_LLVM_LIBS} )

if ( WIN32 AND MINGW )
	set ( SASL_JIT_TEST_LIBS imagehlp psapi )
endif( WIN32 AND MINGW )

ADD_DEFINITIONS(
	-DSASL_USE_LLVM
	-DSALVIA_USE_LLVM
)
