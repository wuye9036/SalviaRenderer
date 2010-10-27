check_build_with_unicode()
set_environment()

include ( tests.cmake )

INCLUDE_DIRECTORIES(
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
	${SASL_HOME_DIR}
)

LINK_DIRECTORIES(
	${BOOST_LIB_DIR}
)

set( SASL_TEST_PROJECT_DIR semantic_test )
set( SASL_TEST_PROJECT_NAME sasl_test_semantic )

configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/test_main.cpp.in
	${SASL_HOME_DIR}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES ${SASL_SEMANTIC_TEST_HEADERS} )
set( SOURCE_FILES test_main.cpp ${SASL_SEMANTIC_TEST_SOURCES} )
ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )

TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	EFLIB
	sasl_enums 
	sasl_common sasl_semantic sasl_syntaxtree
	sasl_test_cases
)

config_output_paths( ${SASL_TEST_PROJECT_NAME} )
sasl_test_create_vcproj_userfile( ${SASL_TEST_PROJECT_NAME} )