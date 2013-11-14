SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SALVIA_LLVM_INCLUDE_DIR}
	${SALVIA_BOOST_INCLUDE_DIR}
)

LINK_DIRECTORIES(
	${SALVIA_BOOST_LIB_DIR}
	${SALVIA_LLVM_LIB_DIR}
)

# Configure Dynamic Link Driver.
ADD_LIBRARY( sasl_drivers
	SHARED
	${SASL_HOME_DIR}/sasl/include/drivers/drivers_api.h
	drivers_api.cpp
	)
	
TARGET_LINK_LIBRARIES( sasl_drivers
	sasl_drivers_lib sasl_shims sasl_codegen sasl_semantic sasl_syntaxtree sasl_parser sasl_enums sasl_common EFLIB
	${SASL_LLVM_LIBS}
	${SALVIA_BOOST_LIBS}
)
SET_TARGET_PROPERTIES( sasl_drivers PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_drivers )

# Configure Static Link Driver.
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/drivers/drivers_forward.h
	
	${SASL_HOME_DIR}/sasl/include/drivers/options.h
	${SASL_HOME_DIR}/sasl/include/drivers/compiler_lib.h
	${SASL_HOME_DIR}/sasl/include/drivers/compiler.h
	${SASL_HOME_DIR}/sasl/include/drivers/compiler_diags.h
	${SASL_HOME_DIR}/sasl/include/drivers/compiler_impl.h
	${SASL_HOME_DIR}/sasl/include/drivers/code_sources.h
)

set( SOURCE_FILES
	compiler_impl.cpp
	compiler_diags.cpp
	compiler_lib.cpp
	options.cpp
	code_sources.cpp
)

ADD_DEFINITIONS( -DSASL_STATIC_DRIVER )
ADD_LIBRARY( sasl_drivers_lib STATIC ${HEADER_FILES} ${SOURCE_FILES} )
SET_TARGET_PROPERTIES( sasl_drivers_lib PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_drivers_lib )