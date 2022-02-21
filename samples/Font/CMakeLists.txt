SALVIA_CHECK_BUILD_WITH_UNICODE()
SET( SOURCE_FILES Font.cpp )
SALVIA_CONFIG_SAMPLE_PROJECT( Font )

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
    EXECUTABLES $<$<TARGET_EXISTS:Font>:$<TARGET_FILE:Font>>
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
