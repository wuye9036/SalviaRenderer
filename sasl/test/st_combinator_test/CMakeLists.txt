check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
	${SASL_HOME_DIR}
)

LINK_DIRECTORIES(
	${BOOST_LIB_DIR}
)

include ( tests.cmake )

set( SASL_TEST_PROJECT_DIR st_combinator_test )
set( SASL_TEST_PROJECT_NAME sasl_test_syntax_tree_combinator )

configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/test_main.cpp.in
	${SASL_HOME_DIR}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/test/test_cases/syntax_cases.h
)

set( SOURCE_FILES
	test_main.cpp
	${SASL_ST_COMBINATOR_TEST_SOURCES}
)

ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	sasl_enums
	sasl_common sasl_syntaxtree 
	sasl_test_cases
)

config_output_paths( ${SASL_TEST_PROJECT_NAME} )
sasl_test_create_vcproj_userfile( ${SASL_TEST_PROJECT_NAME} )