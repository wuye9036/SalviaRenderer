file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "../../include/salvia/ext" "*.h")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "." "*.cpp")

# Platform-specified files
if(UNIX)
  message(STATUS "GL is disabled on Linux. It will be supported later.")
else(UNIX)
  add_definitions(-DSALVIA_EXT_GL_ENABLED)
endif(UNIX)

if( WIN32 )
  set (HEADER_FILES
    ${HEADER_FILES}
    include/utility/inc_windows.h
  )
endif()

if( SALVIA_BUILD_WITH_DIRECTX )
  add_definitions(-DSALVIA_EXT_D3D11_ENABLED)
  
  if(MSVC)
    if(MSVC_VERSION LESS 1700)
      include_directories($ENV{DXSDK_DIR}/Include)
    endif(MSVC_VERSION LESS 1700)
  endif(MSVC)
  
  set (SOURCE_FILES
    ${SOURCE_FILES}
    src/swap_chain/swap_chain_d3d11.cpp
  )
endif()

add_library(salvia_ext STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(salvia_ext PUBLIC "../../include")
target_link_libraries(salvia_ext PRIVATE eflib salvia_resource freeimage::FreeImage freeimage::FreeImagePlus Freetype::Freetype)
set_target_properties(salvia_ext PROPERTIES FOLDER "Salvia")