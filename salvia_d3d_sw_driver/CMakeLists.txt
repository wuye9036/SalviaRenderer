SALVIA_CHECK_BUILD_WITH_UNICODE()

FIND_PATH(DDK_DIR d3dkmthk.h
	PATHS
	"$ENV{ProgramFiles(x86)}/Windows Kits/*/include/km"
	"$ENV{SystemDrive}/WinDDK/*/inc/ddk"
)

IF(DDK_DIR)
	MESSAGE(STATUS "Found DDK in " ${DDK_DIR})

	INCLUDE_DIRECTORIES(
		${SALVIA_HOME_DIR}
		${SALVIA_BOOST_INCLUDE_DIR}
		${DDK_DIR}
		${DDK_DIR}/../um
		${DDK_DIR}/../shared
		include
	)

	LINK_DIRECTORIES(
		${SALVIA_LIBS}
		${SALVIA_BOOST_LIB_DIR}
	)

	set (HEADER_FILES
		include/common.h
		include/display.h
		include/kmd_adapter.h
		include/kmd_context.h
		include/kmd_device.h
		include/umd_adapter.h
		include/umd_device.h
		include/umd_resource.h
	)

	set (SOURCE_FILES
		src/display.cpp
		src/kmd_adapter.cpp
		src/kmd_context.cpp
		src/kmd_device.cpp
		src/main.cpp
		src/thunks.cpp
		src/umd_adapter.cpp
		src/umd_device.cpp
		src/umd_resource.cpp
		
		src/d3d_sw_driver.def
	)

	ADD_LIBRARY( salvia_d3d_sw_driver SHARED
		${HEADER_FILES}	${SOURCE_FILES} )
	TARGET_LINK_LIBRARIES(salvia_d3d_sw_driver salviar EFLIB)
	SET_TARGET_PROPERTIES(salvia_d3d_sw_driver PROPERTIES FOLDER "SALVIA Renderer")
	SALVIA_CONFIG_OUTPUT_PATHS(salvia_d3d_sw_driver)
ENDIF(DDK_DIR)
