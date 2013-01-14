SALVIA_CHECK_BUILD_WITH_UNICODE()

set( ADDITIONAL_INCLUDE_DIRECTORIES
	${SALVIA_LLVM_INCLUDE_PATH}
	)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/shims/ia_shim.h
	${SASL_HOME_DIR}/sasl/include/shims/interp_shim.h
	${SASL_HOME_DIR}/sasl/include/shims/om_shim.h
)

set( SOURCE_FILES
	ia_shim.cpp
	interp_shim.cpp
	om_shim.cpp
)

SASL_CONFIG_LIBRARY(sasl_shims)
