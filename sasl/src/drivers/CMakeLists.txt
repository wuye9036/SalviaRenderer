SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_HOME_DIR}
	${LLVM_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${LLVM_LIBRARY_DIRS}
	${Boost_LIBRARY_DIRS}
)

# Configure Dynamic Link Driver.
add_library( sasl_drivers
	SHARED
	${SASL_HOME_DIR}/sasl/drivers/drivers_api.h
	drivers_api.cpp
	)
	
target_link_libraries( sasl_drivers
	sasl_drivers_lib sasl_shims sasl_codegen sasl_semantic sasl_syntaxtree sasl_parser sasl_enums sasl_common EFLIB
	${Boost_LIBRARIES}
	${LLVM_LIBS}
)
set_target_properties(sasl_drivers PROPERTIES FOLDER "Shader Drivers")

install(TARGETS sasl_drivers)
install(
    CODE [[
  SET(EXCLUDE_PATTERNS
	"^libcdpl.*"
	"^.*linux.*"
	"^libGL.*"
	"^libX.*"
	"^libpython.*"
    "^.*system32.*"
    "^.*ms-win-core.*"
  )

  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES $<$<TARGET_EXISTS:sasl_drivers>:$<TARGET_FILE:sasl_drivers>>
    RESOLVED_DEPENDENCIES_VAR _r_deps
    UNRESOLVED_DEPENDENCIES_VAR _u_deps
    POST_EXCLUDE_REGEXES ${EXCLUDE_PATTERNS}
  )
  file(INSTALL ${_r_deps}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    FOLLOW_SYMLINK_CHAIN
  )
  message(STATUS "Resolved deps: ${_r_deps}")
  message(STATUS "Unresolved deps: ${_u_deps}")
]]
)
# Configure Static Link Driver.
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/drivers/drivers_forward.h
	
	${SASL_HOME_DIR}/sasl/drivers/options.h
	${SASL_HOME_DIR}/sasl/drivers/compiler_lib.h
	${SASL_HOME_DIR}/sasl/drivers/compiler.h
	${SASL_HOME_DIR}/sasl/drivers/compiler_diags.h
	${SASL_HOME_DIR}/sasl/drivers/compiler_impl.h
	${SASL_HOME_DIR}/sasl/drivers/code_sources.h
)

set( SOURCE_FILES
	compiler_impl.cpp
	compiler_diags.cpp
	compiler_lib.cpp
	options.cpp
	code_sources.cpp
)

add_definitions( -DSASL_STATIC_DRIVER )
add_library(sasl_drivers_lib STATIC ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties( sasl_drivers_lib PROPERTIES FOLDER "Shader Drivers")