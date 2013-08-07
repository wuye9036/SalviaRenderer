SALVIA_CHECK_BUILD_WITH_UNICODE()

IF(CMAKE_GENERATOR MATCHES "Win64")
	SET(SALVIA_ARCH_NAME "x64")
ELSE()
	SET(SALVIA_ARCH_NAME "x86")
ENDIF()

INCLUDE_DIRECTORIES(
	$ENV{DXSDK_DIR}/Include
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_FREETYPE_INCLUDE_DIR}
)
LINK_DIRECTORIES(
	$ENV{DXSDK_DIR}/Lib/${SALVIA_ARCH_NAME}
	${SALVIA_3RD_PARTY_LIBS}
	${SALVIA_FREETYPE_LIB_DIR}
	${SALVIA_BOOST_LIB_DIR}
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
	include/resource/texture/sa/tex_io.h
	include/resource/texture/freeimage/tex_io_freeimage.h
	include/resource/font/font.h
	
	include/resource/terrain/gen_terrain.h
	
	include/utility/freeimage_utilities.h
	include/utility/user_config.h
	include/utility/utility_forward.h
)

set (SOURCE_FILES
	src/resource/mesh/sa/material.cpp
	src/resource/mesh/sa/mesh_impl.cpp
	src/resource/mesh/sa/skin_mesh_impl.cpp
	src/resource/mesh/sa/mesh_io.cpp
	src/resource/mesh/sa/mesh_io_obj.cpp
	src/resource/mesh/sa/mesh_io_collada.cpp
	src/resource/mesh/sa/collada.cpp
	
	src/resource/texture/freeimage/tex_io_freeimage.cpp
	src/resource/font/font.cpp
	src/resource/terrain/gen_terrain.cpp
	
	src/utility/freeimage_utility.cpp
)

if( WIN32 )
	set (HEADER_FILES ${HEADER_FILES}
		include/utility/inc_gdiplus.h
		include/utility/inc_windows.h
		)
	set (SOURCE_FILES ${SOURCE_FILES}
		src/resource/texture/gdiplus/tex_io_gdiplus.cpp
	)
endif()

ADD_LIBRARY( salviax STATIC	${HEADER_FILES}	${SOURCE_FILES} )
TARGET_LINK_LIBRARIES(salviax EFLIB	salviar FreeImage freetype)
SET_TARGET_PROPERTIES(salviax PROPERTIES FOLDER "SALVIA Renderer")
SALVIA_CONFIG_OUTPUT_PATHS(salviax)

# Swap Chain
ADD_DEFINITIONS(-DSALVIAX_GL_ENABLED)
set (HEADER_FILES
	include/swap_chain/swap_chain.h
	include/swap_chain/swap_chain_impl.h
)
set (SOURCE_FILES
	src/swap_chain/swap_chain_impl.cpp
	src/swap_chain/swap_chain_gl.cpp
)
# D3D11
if( SALVIA_BUILD_WITH_DIRECTX )
	ADD_DEFINITIONS(-DSALVIAX_D3D11_ENABLED)
	set (SOURCE_FILES
		${SOURCE_FILES}
		src/swap_chain/swap_chain_d3d11.cpp
		src/swap_chain/swap_chain_d3d11.rc
		src/swap_chain/vs.hlsl
		src/swap_chain/ps.hlsl
	)
endif()

ADD_LIBRARY			 (salviax_swap_chain SHARED ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES(salviax_swap_chain EFLIB salviar)
SET_TARGET_PROPERTIES(salviax_swap_chain PROPERTIES FOLDER "SALVIA Renderer")
SALVIA_CONFIG_OUTPUT_PATHS(salviax_swap_chain)