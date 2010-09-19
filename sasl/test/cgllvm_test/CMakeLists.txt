check_build_with_unicode()
set_environment()

include (tests.cmake)
INCLUDE_DIRECTORIES(
	${boost_home_dir}
	${softart_home_dir}
	${sasl_home_dir}
	${3rd_party_llvm_include_path}
)

LINK_DIRECTORIES(
	${boost_lib_dir}
	${3rd_party_llvm_lib_path}
)

set( SASL_TEST_PROJECT_DIR cgllvm_test )
set( SASL_TEST_PROJECT_NAME sasl_test_cgllvm )

configure_file(
	${sasl_home_dir}/sasl/test/test_resources/test_main.cpp.in
	${sasl_home_dir}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES ${SASL_CGLLVM_TEST_HEADERS} )
set( SOURCE_FILES test_main.cpp ${SASL_CGLLVM_TEST_SOURCES} )

ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	sasl_enums sasl_common
	sasl_semantic sasl_syntaxtree
	sasl_test_cases
	${SASL_CGLLVM_TEST_LIBS}
)

config_output_paths( sasl_test_cgllvm )