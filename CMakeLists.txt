project(salvia)

cmake_minimum_required(VERSION 3.21)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure dependencies
find_package(range-v3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)
find_package(Freetype REQUIRED)
# find_package(magic_enum CONFIG REQUIRED)
find_path(SIMDE_INCLUDE_DIRS "simde/arm/neon.h")

include(cmake/IncludeBoost.cmake)
include(cmake/IncludeLLVM.cmake)
include(cmake/ConfigCompiler.cmake)

add_subdirectory(cmake)

# Add projects
add_subdirectory(eflib)
add_subdirectory(salvia)
add_subdirectory(sasl)
add_subdirectory(samples)

# if( SALVIA_BUILD_SW_DRIVER )
# 	add_subdirectory( salvia_d3d_sw_driver )
# endif()

enable_testing()
add_test(NAME Test-Shader COMMAND $<TARGET_FILE:sasl_test>)
add_test(NAME Test-EFLib COMMAND $<TARGET_FILE:eflib_test>)
add_test(NAME Test-Salvia COMMAND $<TARGET_FILE:salvia_test>)

add_custom_target(
        copy-compile-commands ALL
        ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_LIST_DIR}
)