file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/drivers/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

include(CheckCXXCompilerFlag)

# Configure Dynamic Link Driver.
add_library( sasl_drivers ${HEADER_LIST} ${SOURCE_LIST})
# disable compiler error caused by __DATE__ and __TIME__ which is used in Boost.Wave
target_compile_options_if_applicable(sasl_drivers "-Wno-date-time")
target_include_directories(sasl_drivers PUBLIC ../../include)
target_link_libraries( sasl_drivers
	sasl_shims sasl_codegen sasl_semantic sasl_syntaxtree sasl_parser sasl_enums sasl_common eflib)


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