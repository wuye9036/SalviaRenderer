SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_LLVM_INCLUDE_PATH}
	${SALVIA_HOME_DIR}
)

LINK_DIRECTORIES(
	${SALVIA_BOOST_LIB_DIR}
	${SALVIA_LLVM_LIB_PATH}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/host/host.h
	${SALVIA_HOME_DIR}/salviar/include/shader_enums.h
)

set( SOURCE_FILES
	host.cpp
)

ADD_LIBRARY( sasl_host SHARED
	${HEADER_FILES} ${SOURCE_FILES}
)

TARGET_LINK_LIBRARIES( sasl_host
	EFLIB
	sasl_semantic sasl_syntaxtree
	sasl_parser
	sasl_enums sasl_common
	sasl_cgllvm
	${SASL_LLVM_LIBS}
)

SALVIA_CONFIG_OUTPUT_PATHS( sasl_host )

