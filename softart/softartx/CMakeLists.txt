ADD_DEFINITIONS( -DWIN32
-D_LIB
-D_CRT_SECURE_NO_DEPRECATE
-D_SCL_SECURE_NO_DEPRECATE
-D_SECURE_SCL=0
-DBOOST_ALL_DYN_LINK
)
check_build_with_unicode()

INCLUDE_DIRECTORIES(../
../../3rd_party/threadpool
../../3rd_party/FreeImage/include
)

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
	#SET (USE_GDIPLUS_LIBRARY CACHE BOOL "Should we use the GDIPLUS.dll (better drawing)")  
	#IF (USE_GDIPLUS_LIBRARY) 
	#MESSAGE ("Using gdiplus.dll for better drawings") 
	TARGET_LINK_LIBRARIES (SoftArtX "gdiplus") 
	#ENDIF (USE_GDIPLUS_LIBRARY) 
ENDIF (WIN32) 


config_output_paths()