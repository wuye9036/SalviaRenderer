set( SASL_ABI_TEST_HEADERS	"" )
set( SASL_ABI_TEST_SOURCES	"" )
set( SASL_ABI_TEST_LIBS 	"" )

set( SASL_ABI_TEST_HEADERS	${SASL_HOME_DIR}/sasl/test/abi_test/abi_test.h  )
set( SASL_ABI_TEST_SOURCES	${SASL_HOME_DIR}/sasl/test/abi_test/abi_test.cpp)
set( SASL_ABI_TEST_LIBS		"" )

if ( WIN32 AND MINGW )
	set ( SASL_ABI_TEST_LIBS imagehlp psapi )
endif( WIN32 AND MINGW )
