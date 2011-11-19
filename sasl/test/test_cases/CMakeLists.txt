SALVIA_CHECK_BUILD_WITH_UNICODE()

include ( llvm_test_cases.cmake )

INCLUDE_DIRECTORIES(
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SASL_HOME_DIR}
)

set( HEADER_FILES
	utility.h
	syntax_cases.h
	semantic_cases.h
	${SALVIA_LLVM_TEST_CASE_HEADERS}
)

set( SOURCE_FILES
	syntax_cases.cpp
	semantic_cases.cpp
	${SALVIA_LLVM_TEST_CASE_SOURCES}
)

SOURCE_GROUP( "Header Files" FILES ${HEADER_FILES} )
SOURCE_GROUP( "Source Files" FILES ${SOURCE_FILES} )
include( ${SASL_HOME_DIR}/sasl/test/auto_test/repo/question/tests.cmake )

ADD_LIBRARY( sasl_test_cases ${HEADER_FILES} ${SOURCE_FILES} ${V1A1_TESTS} )

if (MSVC)
	Set( SALVIA_TEST_REPOSITORY_PATH
		"${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/repo"
		)
else( MSVC )
	set( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/repo" )
endif()

ADD_CUSTOM_COMMAND(
		TARGET sasl_test_cases
		PRE_BUILD
		COMMAND "python.exe"
		ARGS
			"${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py"
			"${SASL_HOME_DIR}/sasl/test/auto_test/repo"
			${SALVIA_TEST_REPOSITORY_PATH}
		DEPENDS
			"${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py"
			${V1A1_TESTS}
		)
		
SALVIA_CONFIG_OUTPUT_PATHS( sasl_test_cases )