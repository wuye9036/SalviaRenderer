SALVIA_CHECK_BUILD_WITH_UNICODE()

include (tests.cmake)
INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_HOME_DIR}
	${LLVM_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

MESSAGE(STATUS ${LLVM_LIBRARY_DIRS})

LINK_DIRECTORIES(
	${LLVM_LIBRARY_DIRS}
	${Boost_LIBRARY_DIRS}
)

set( SASL_TEST_PROJECT_DIR jit_test )
set( SASL_TEST_PROJECT_NAME sasl_test_jit )

configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/test_main.cpp.in
	${SASL_HOME_DIR}/sasl/test/${SASL_TEST_PROJECT_DIR}/test_main.cpp
	@ONLY
)

set( HEADER_FILES ${SASL_JIT_TEST_HEADERS} )
set( SOURCE_FILES test_main.cpp ${SASL_JIT_TEST_SOURCES} )

ADD_EXECUTABLE( ${SASL_TEST_PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( ${SASL_TEST_PROJECT_NAME}
	sasl_drivers
	sasl_semantic sasl_syntaxtree
	sasl_parser
	sasl_enums sasl_common
	${Boost_LIBRARIES}
	${LLVM_LIBS}
)
ADD_DEPENDENCIES(${SASL_TEST_PROJECT_NAME} sasl_test_shader_repo)

SET_TARGET_PROPERTIES( ${SASL_TEST_PROJECT_NAME} PROPERTIES FOLDER "Shader Tests")
SASL_TEST_CREATE_VCPROJ_USERFILE( ${SASL_TEST_PROJECT_NAME} )

install(TARGETS sasl_test_jit)