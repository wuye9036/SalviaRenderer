check_build_with_unicode()
set_environment()

include ( ../cgllvm_test/tests.cmake )
include ( ../st_combinator_test/tests.cmake )
include ( ../semantic_test/tests.cmake )

INCLUDE_DIRECTORIES(
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
	${SASL_HOME_DIR}
	${3rd_party_llvm_include_path}
)

LINK_DIRECTORIES(
	${BOOST_LIB_DIR}
	${3rd_party_llvm_lib_path}
)

set( SASL_TEST_PROJECT_DIR regression_test )
set( SASL_TEST_PROJECT_NAME sasl_test_regression )
configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/test_main.cpp.in
	${SASL_HOME_DIR}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES "" )
set( SOURCE_FILES test_main.cpp
	${SASL_CGLLVM_TEST_SOURCES}
	${SASL_ST_COMBINATOR_TEST_SOURCES}
	${SASL_SEMANTIC_TEST_SOURCES}
)

ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	EFLIB
	sasl_test_cases
	sasl_semantic sasl_syntaxtree
	sasl_enums sasl_common
	${SASL_CGLLVM_TEST_LIBS}
	${SOFTART_BOOST_LIBS}
)
ADD_DEPENDENCIES( ${SASL_TEST_PROJECT_NAME} logrout )

config_output_paths( ${SASL_TEST_PROJECT_NAME} )
sasl_test_create_vcproj_userfile( ${SASL_TEST_PROJECT_NAME} )
