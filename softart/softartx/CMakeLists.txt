check_build_with_unicode()
config_3rd_party()
INCLUDE_DIRECTORIES(../
${3rd_party_includes}
)
LINK_DIRECTORIES(${3rd_party_libs})

set (HEADER_FILES
	include/resource/mesh/sa/mesh.h
	include/resource/mesh/sa/mesh_io.h
	include/resource/mesh/dx9/mesh_io_d3d9.h
	include/resource/resource_forward.h
	include/resource/texture/sa/tex_io.h
	include/resource/texture/freeimage/tex_io_freeimage.h
	include/resource/texture/gdiplus/tex_io_gdiplus.h
	include/utility/d3d9_utilities.h
	include/utility/freeimage_utilities.h
	include/utility/inc_d3d9.h
	include/utility/inc_d3d9x.h
	include/utility/inc_gdiplus.h
	include/utility/inc_windows.h
	include/utility/user_config.h
	include/utility/utility_forward.h
	include/presenter/sa/dev.h
	include/presenter/dx9/dev_d3d9.h
	include/presenter/gdiplus/dev_gdiplus.h
)

set (SOURCE_FILES
	src/presenter/dx9/dev_d3d9.cpp
	src/presenter/gdiplus/dev_gdiplus.cpp
	src/resource/mesh/sa/mesh.cpp
	src/resource/mesh/sa/mesh_io.cpp
	src/resource/mesh/dx9/mesh_io_d3d9.cpp
	src/resource/texture/sa/tex_io.cpp
	src/resource/texture/freeimage/tex_io_freeimage.cpp
	src/resource/texture/gdiplus/tex_io_gdiplus.cpp
	src/utility/d3d9_utilities.cpp
	src/utility/freeimage_utility.cpp
)

ADD_LIBRARY( SoftArtX STATIC
${HEADER_FILES}
${SOURCE_FILES}
)

TARGET_LINK_LIBRARIES(SoftArtX
EFLIB
SoftArt
)

IF (WIN32) 
	TARGET_LINK_LIBRARIES (SoftArtX "gdiplus") 
ENDIF (WIN32) 

TARGET_LINK_LIBRARIES (SoftArtX FreeImage) 


config_output_paths(SoftArtX)