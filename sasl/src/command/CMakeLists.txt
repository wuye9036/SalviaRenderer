SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SALVIA_LLVM_INCLUDE_PATH}
)

LINK_DIRECTORIES(
	${SALVIA_BOOST_LIB_DIR}
	${SALVIA_LLVM_LIB_PATH}
)

set( SOURCE_FILES
	command.cpp
)

ADD_EXECUTABLE( sasl_command ${HEADER_FILES} ${SOURCE_FILES} )

TARGET_LINK_LIBRARIES( sasl_command
	sasl_compiler sasl_cgllvm
	sasl_semantic sasl_syntaxtree sasl_parser
	sasl_enums sasl_common
	${SASL_LLVM_LIBS}
	${SALVIA_BOOST_LIBS}
)

SET_TARGET_PROPERTIES( sasl_command PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_command )

