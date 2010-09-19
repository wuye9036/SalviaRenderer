check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${boost_home_dir}
	${softart_home_dir}
	${sasl_home_dir}
)

LINK_DIRECTORIES(
	${boost_lib_dir}
)

include ( tests.cmake )

set( SASL_TEST_PROJECT_DIR st_combinator_test )
set( SASL_TEST_PROJECT_NAME sasl_test_syntax_tree_combinator )

configure_file(
	${sasl_home_dir}/sasl/test/test_resources/test_main.cpp.in
	${sasl_home_dir}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES
	${sasl_home_dir}/sasl/test/test_cases/syntax_cases.h
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