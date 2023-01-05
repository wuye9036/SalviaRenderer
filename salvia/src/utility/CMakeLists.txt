file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "../../include/salvia/utility/*.h")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "*.cpp" "*.h")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/../../include/salvia/utility" PREFIX "Header Files" FILES ${HEADER_LIST})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" PREFIX "Source Files" FILES ${SOURCE_LIST})

add_library(salvia_utility SHARED ${HEADER_LIST} ${SOURCE_LIST})

target_include_directories(salvia_utility PUBLIC "../../include")
target_link_libraries(salvia_utility
        salvia_ext salvia_core salvia_resource salvia_shader
        eflib fmt::fmt Boost::program_options
        )

#install(TARGETS salviau)
#install(
#    CODE [[
#  SET(EXCLUDE_PATTERNS
#	"^libcdpl.*"
#	"^.*linux.*"
#	"^libGL.*"
#	"^libX.*"
#	"^libpython.*"
#    "^.*system32.*"
#    "^.*ms-win-core.*"
#  )
#
#  file(GET_RUNTIME_DEPENDENCIES
#    EXECUTABLES $<$<TARGET_EXISTS:salviau>:$<TARGET_FILE:salviau>>
#    RESOLVED_DEPENDENCIES_VAR _r_deps
#    UNRESOLVED_DEPENDENCIES_VAR _u_deps
#    POST_EXCLUDE_REGEXES ${EXCLUDE_PATTERNS}
#  )
#  file(INSTALL ${_r_deps}
#    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
#    FOLLOW_SYMLINK_CHAIN
#  )
#  message(STATUS "Resolved deps: ${_r_deps}")
#  message(STATUS "Unresolved deps: ${_u_deps}")
#]]
#)
