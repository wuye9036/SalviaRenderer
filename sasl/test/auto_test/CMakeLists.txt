configure_file(
	${SASL_HOME_DIR}/sasl/test/test_resources/auto_test_config.in
	${SASL_HOME_DIR}/sasl/test/auto_test/src/config.py
	@ONLY
)

SALVIA_CHECK_BUILD_WITH_UNICODE()

# Add test repos.

include( ${SASL_HOME_DIR}/sasl/test/auto_test/repo/question/tests.cmake )

if (MSVC)
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/repo" )
else( MSVC )
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/repo" )
endif()

ADD_CUSTOM_COMMAND(
		OUTPUT	auto_repo.deps
		COMMAND "${PYTHON_EXECUTABLE}"
		ARGS
			"${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py"
			"${SASL_HOME_DIR}/sasl/test/auto_test/repo"
			${SALVIA_TEST_REPOSITORY_PATH}
		COMMAND echo "AUTO_REPO.DEPS" > auto_repo.deps
		DEPENDS
			"${SASL_HOME_DIR}/sasl/test/auto_test/src/copy_tests.py" ${V1A1_TESTS}
			"${SASL_HOME_DIR}/sasl/test/auto_test/repo/question/tests.cmake"
		)
ADD_CUSTOM_TARGET( sasl_test_repo ALL DEPENDS auto_repo.deps ${V1A1_TESTS} SOURCES ${V1A1_TESTS} )
SET_TARGET_PROPERTIES( sasl_test_repo PROPERTIES FOLDER "Shader Tests")