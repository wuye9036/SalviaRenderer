project(salvia)

cmake_minimum_required(VERSION 3.21)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(SALVIA_HOME_DIR ${CMAKE_HOME_DIRECTORY})

# Configure dependencies
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)
# find_package(magic_enum CONFIG REQUIRED)
include (cmake/IncludeBoost.cmake)
include (cmake/IncludeLLVM.cmake)

include (cmake/ConfigCompiler.cmake)
add_subdirectory( cmake )

# Add projects
add_subdirectory(eflib)
add_subdirectory(shader)
add_subdirectory(sasl)
# add_subdirectory(salviar)
# add_subdirectory(salviax)
# add_subdirectory(salviau)
# add_subdirectory(samples)

# if( SALVIA_BUILD_SW_DRIVER )
# 	add_subdirectory( salvia_d3d_sw_driver )
# endif()

enable_testing()
add_test(NAME ShaderTests COMMAND $<TARGET_FILE:sasl_test>)