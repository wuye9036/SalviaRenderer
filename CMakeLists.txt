PROJECT( salvia )

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
set( SALVIA_HOME_DIR ${CMAKE_HOME_DIRECTORY} )

# Configure dependencies
find_package(Boost REQUIRED COMPONENTS log program_options unit_test_framework wave thread date_time locale chrono filesystem atomic system)

if (MSVC)
	list(APPEND Boost_LIBRARIES bcrypt.lib)
endif()

find_package(freeimage CONFIG REQUIRED)

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

# include (${CMAKE_HOME_DIRECTORY}/cmake/SystemInfo.cmake)

include (${CMAKE_HOME_DIRECTORY}/cmake/ConfigCompiler.cmake)

# include (${CMAKE_HOME_DIRECTORY}/cmake/Variables.cmake)
# include (${CMAKE_HOME_DIRECTORY}/cmake/ConfigurePath.cmake)
ADD_SUBDIRECTORY( cmake )

# Add projects
ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( sasl )
# ADD_SUBDIRECTORY( salviar )
# ADD_SUBDIRECTORY( salviax )
# ADD_SUBDIRECTORY( salviau )
# ADD_SUBDIRECTORY( samples )
# if( SALVIA_BUILD_SW_DRIVER )
# 	ADD_SUBDIRECTORY( salvia_d3d_sw_driver )
# endif()

