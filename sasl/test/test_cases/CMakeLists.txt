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
	${SALVIA_LLVM_TEST_CASE_HEADERS}
)

set( SOURCE_FILES
	syntax_cases.cpp
	${SALVIA_LLVM_TEST_CASE_SOURCES}
)

SOURCE_GROUP( "Header Files" FILES ${HEADER_FILES} )
SOURCE_GROUP( "Source Files" FILES ${SOURCE_FILES} )

ADD_LIBRARY( sasl_test_cases ${HEADER_FILES} ${SOURCE_FILES}  )
SET_TARGET_PROPERTIES( sasl_test_cases PROPERTIES FOLDER "Shader Tests")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_test_cases )

# Add test repos.

include( ${SASL_HOME_DIR}/sasl/test/auto_test/repo/question/tests.cmake )

if (MSVC)
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/repo" )
else( MSVC )
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/repo" )
endif()

ADD_CUSTOM_COMMAND(
		OUTPUT	auto_repo.deps
		COMMAND "${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py"
		ARGS
			"${SASL_HOME_DIR}/sasl/test/auto_test/repo" ${SALVIA_TEST_REPOSITORY_PATH}
		COMMAND echo "AUTO_REPO.DEPS" > auto_repo.deps
		DEPENDS
			"${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py" ${V1A1_TESTS}
			"${SASL_HOME_DIR}/sasl/test/auto_test/repo/question/tests.cmake"
		)
ADD_CUSTOM_TARGET( sasl_test_repo ALL DEPENDS auto_repo.deps ${V1A1_TESTS} SOURCES ${V1A1_TESTS} )
SET_TARGET_PROPERTIES( sasl_test_repo PROPERTIES FOLDER "Shader Tests")