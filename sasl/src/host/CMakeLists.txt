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

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/host/host_impl.h
	${SASL_HOME_DIR}/sasl/include/host/shader_log_impl.h
	${SASL_HOME_DIR}/sasl/include/host/shader_object_impl.h
)

set( SOURCE_FILES
	host_impl.cpp
	shader_log_impl.cpp
	shader_object_impl.cpp
)

ADD_LIBRARY( sasl_host SHARED
	${HEADER_FILES} ${SOURCE_FILES}
)

TARGET_LINK_LIBRARIES( sasl_host EFLIB sasl_driver )
SET_TARGET_PROPERTIES( sasl_host PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_host )

