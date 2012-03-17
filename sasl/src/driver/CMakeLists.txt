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

# Config Dynamic Link Driver.
ADD_LIBRARY( sasl_driver SHARED ${SASL_HOME_DIR}/sasl/include/driver/driver_api.h driver_api.cpp )
TARGET_LINK_LIBRARIES( sasl_driver
	EFLIB sasl_semantic sasl_syntaxtree sasl_parser sasl_enums sasl_common sasl_cgllvm sasl_driver_lib
	${SASL_LLVM_LIBS}
	${SALVIA_BOOST_LIBS}
)
SET_TARGET_PROPERTIES( sasl_driver PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_driver )

# Config Static Link Driver.

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/driver/options.h
	${SASL_HOME_DIR}/sasl/include/driver/driver_lib.h
	${SASL_HOME_DIR}/sasl/include/driver/driver.h
	${SASL_HOME_DIR}/sasl/include/driver/driver_impl.h
	${SASL_HOME_DIR}/sasl/include/driver/driver_forward.h
	${SASL_HOME_DIR}/sasl/include/driver/code_sources.h
)

set( SOURCE_FILES
	driver_impl.cpp
	driver_lib.cpp
	options.cpp
	code_sources.cpp
)

ADD_DEFINITIONS( -DSASL_STATIC_DRIVER )
ADD_LIBRARY( sasl_driver_lib STATIC ${HEADER_FILES} ${SOURCE_FILES} )
SET_TARGET_PROPERTIES( sasl_driver_lib PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_driver_lib )