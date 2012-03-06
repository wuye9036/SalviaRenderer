SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
)
LINK_DIRECTORIES(${SALVIA_3RD_PARTY_LIBS})

set (HEADER_FILES
	include/resource/mesh/sa/mesh.h
	include/resource/mesh/sa/mesh_io.h
	include/resource/mesh/sa/mesh_io_obj.h
	include/resource/resource_forward.h
	include/resource/texture/sa/tex_io.h
	include/resource/texture/freeimage/tex_io_freeimage.h
	include/resource/texture/gdiplus/tex_io_gdiplus.h
	include/utility/freeimage_utilities.h
	include/utility/user_config.h
	include/utility/utility_forward.h
)
set (SOURCE_FILES
	src/resource/mesh/sa/mesh.cpp
	src/resource/mesh/sa/mesh_io.cpp
	src/resource/mesh/sa/mesh_io_obj.cpp
	
	src/resource/texture/sa/tex_io.cpp
	src/resource/texture/freeimage/tex_io_freeimage.cpp
	
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

if( SALVIA_BUILD_WITH_DIRECTX )
	set (HEADER_FILES ${HEADER_FILES}
		include/utility/d3d9_utilities.h
		include/utility/inc_d3d9.h
		include/utility/inc_d3d9x.h
		include/resource/mesh/dx9/mesh_io_d3d9.h
		)
	set (SOURCE_FILES ${SOURCE_FILES}
		src/resource/mesh/dx9/mesh_io_d3d9.cpp
		src/utility/d3d9_utilities.cpp
		)
	ADD_DEFINITIONS( -DSALVIA_BULD_WITH_DIRECTX )
endif()

ADD_LIBRARY( salviax STATIC	${HEADER_FILES}	${SOURCE_FILES} )
TARGET_LINK_LIBRARIES(salviax EFLIB	salviar FreeImage)
SET_TARGET_PROPERTIES(salviax PROPERTIES FOLDER "Core Renderer")
SALVIA_CONFIG_OUTPUT_PATHS(salviax)

if( SALVIA_BUILD_WITH_DIRECTX )
	# D3D9
	set (HEADER_FILES include/presenter/d3d9/dev_d3d9.h)
	set (SOURCE_FILES src/presenter/d3d9/dev_d3d9.cpp)
	ADD_LIBRARY( salviax_d3d9_presenter SHARED
		${HEADER_FILES}
		${SOURCE_FILES}
	)
	TARGET_LINK_LIBRARIES(salviax_d3d9_presenter EFLIB salviar salviax)
	SET_TARGET_PROPERTIES(salviax_d3d9_presenter PROPERTIES FOLDER "Presenters")
	SALVIA_CONFIG_OUTPUT_PATHS(salviax_d3d9_presenter)

	# D3D11
	set (HEADER_FILES include/presenter/d3d11/dev_d3d11.h)
	set (SOURCE_FILES
		src/presenter/d3d11/dev_d3d11.cpp
		src/presenter/d3d11/dev_d3d11.rc
		src/presenter/d3d11/vs.hlsl
		src/presenter/d3d11/ps.hlsl
	)
	ADD_LIBRARY( salviax_d3d11_presenter SHARED
		${HEADER_FILES}
		${SOURCE_FILES}
	)
	TARGET_LINK_LIBRARIES(salviax_d3d11_presenter EFLIB	salviar)
	SET_TARGET_PROPERTIES(salviax_d3d11_presenter PROPERTIES FOLDER "Presenters")
	SALVIA_CONFIG_OUTPUT_PATHS(salviax_d3d11_presenter)
endif()

IF (WIN32)
	set (HEADER_FILES include/presenter/gdiplus/dev_gdiplus.h)
	set (SOURCE_FILES src/presenter/gdiplus/dev_gdiplus.cpp)
	ADD_LIBRARY( salviax_gdiplus_presenter SHARED
		${HEADER_FILES}
		${SOURCE_FILES}
	)
	TARGET_LINK_LIBRARIES(salviax_gdiplus_presenter EFLIB salviar salviax "gdiplus")
	SET_TARGET_PROPERTIES(salviax_gdiplus_presenter PROPERTIES FOLDER "Presenters")
	SALVIA_CONFIG_OUTPUT_PATHS(salviax_gdiplus_presenter)
ENDIF (WIN32) 

set (HEADER_FILES include/presenter/opengl/dev_opengl.h)
set (SOURCE_FILES src/presenter/opengl/dev_opengl.cpp)
ADD_LIBRARY( salviax_opengl_presenter SHARED
	${HEADER_FILES}
	${SOURCE_FILES}
)
TARGET_LINK_LIBRARIES(salviax_opengl_presenter EFLIB salviar )
SET_TARGET_PROPERTIES(salviax_opengl_presenter PROPERTIES FOLDER "Presenters")
SALVIA_CONFIG_OUTPUT_PATHS(salviax_opengl_presenter)
