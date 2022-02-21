project(salvia)

cmake_minimum_required(VERSION 3.21)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(SALVIA_HOME_DIR ${CMAKE_HOME_DIRECTORY})

# Configure dependencies
find_package(
    Boost REQUIRED COMPONENTS
    log
    program_options
    unit_test_framework
    wave
    thread
    date_time
    locale
    chrono
    filesystem
    atomic
    system
)
# Workaround for linking failure of Boost.UUID
if (MSVC)
	list(APPEND Boost_LIBRARIES bcrypt.lib)
endif()
find_package(llvm CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS
  Analysis
  Core
  ExecutionEngine
  InstCombine
  Object
  OrcJIT
  RuntimeDyld
  ScalarOpts
  Support
  native
  MCJIT
  X86CodeGen
)
find_package(freeimage CONFIG REQUIRED)

include (cmake/ConfigCompiler.cmake)
add_subdirectory( cmake )

# Add projects
add_subdirectory(eflib)
add_subdirectory(sasl)
add_subdirectory(salviar)
add_subdirectory(salviax)
add_subdirectory(salviau)
add_subdirectory(samples)

# if( SALVIA_BUILD_SW_DRIVER )
# 	add_subdirectory( salvia_d3d_sw_driver )
# endif()

