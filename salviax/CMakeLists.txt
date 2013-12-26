SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_BOOST_INCLUDE_DIR}
	${SALVIA_FREETYPE_INCLUDE_DIR}
	${SALVIA_FREEIMAGE_INCLUDE_DIR}
)

set (HEADER_FILES
	include/resource/mesh/sa/material.h
	include/resource/mesh/sa/mesh.h
	include/resource/mesh/sa/mesh_impl.h
	include/resource/mesh/sa/skin_mesh_impl.h
	include/resource/mesh/sa/collada.h
	
	include/resource/mesh/sa/mesh_io.h
	include/resource/mesh/sa/mesh_io_obj.h
	include/resource/mesh/sa/mesh_io_collada.h
	
	include/resource/resource_forward.h
	include/resource/texture/tex_io.h
	include/resource/font/font.h
	
	include/resource/terrain/gen_terrain.h
	
	include/utility/freeimage_utilities.h
	include/utility/utility_forward.h
	
	include/swap_chain/swap_chain.h
	include/swap_chain/swap_chain_impl.h
)

set (SOURCE_FILES
	src/resource/mesh/sa/material.cpp
	src/resource/mesh/sa/mesh_impl.cpp
	src/resource/mesh/sa/skin_mesh_impl.cpp
	src/resource/mesh/sa/mesh_io.cpp
	src/resource/mesh/sa/mesh_io_obj.cpp
	src/resource/mesh/sa/mesh_io_collada.cpp
	src/resource/mesh/sa/collada.cpp
	
	src/resource/texture/tex_io.cpp
	src/resource/font/font.cpp
	src/resource/terrain/gen_terrain.cpp
	
	src/utility/freeimage_utility.cpp
	
	src/swap_chain/swap_chain_impl.cpp
	src/swap_chain/swap_chain_gl.cpp
)

# Platform-specified files
ADD_DEFINITIONS(-DSALVIAX_GL_ENABLED)

if( WIN32 )
	set (HEADER_FILES
		${HEADER_FILES}
		include/utility/inc_windows.h
	)
endif()

if( SALVIA_BUILD_WITH_DIRECTX )
	ADD_DEFINITIONS(-DSALVIAX_D3D11_ENABLED)
	
	if(MSVC)
		if(MSVC_VERSION LESS 1700)
			INCLUDE_DIRECTORIES($ENV{DXSDK_DIR}/Include)
		endif(MSVC_VERSION LESS 1700)
	endif(MSVC)
	
	set (SOURCE_FILES
		${SOURCE_FILES}
		src/swap_chain/swap_chain_d3d11.cpp
	)
endif()

ADD_LIBRARY( salviax STATIC	${HEADER_FILES}	${SOURCE_FILES} )
TARGET_LINK_LIBRARIES(salviax EFLIB	salviar FreeImage freetype)
SET_TARGET_PROPERTIES(salviax PROPERTIES FOLDER "SALVIA Renderer")
SALVIA_CONFIG_OUTPUT_PATHS(salviax)