SALVIA_CHECK_BUILD_WITH_UNICODE()

include (tests.cmake)
INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_HOME_DIR}
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${Boost_LIBRARY_DIR}
)

set( SASL_TEST_PROJECT_DIR abi_test )
set( SASL_TEST_PROJECT_NAME sasl_test_abi )

configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/test_main.cpp.in
	${SASL_HOME_DIR}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES ${SASL_ABI_TEST_HEADERS} )
set( SOURCE_FILES test_main.cpp ${SASL_ABI_TEST_SOURCES} )

ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	sasl_semantic sasl_syntaxtree
	sasl_parser
	sasl_enums sasl_common
	${SASL_ABI_TEST_LIBS}
	${Boost_LIBRARIES}
)
ADD_DEPENDENCIES(${SASL_TEST_PROJECT_NAME} sasl_test_shader_repo)

SET_TARGET_PROPERTIES( ${SASL_TEST_PROJECT_NAME} PROPERTIES FOLDER "Shader Tests")
# SASL_TEST_CREATE_VCPROJ_USERFILE( ${SASL_TEST_PROJECT_NAME} )
