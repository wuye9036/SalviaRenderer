SALVIA_CHECK_BUILD_WITH_UNICODE()

FIND_PATH(DDK_DIR d3dkmthk.h
	PATHS
	"$ENV{ProgramFiles(x86)}/Windows Kits/*/include/km"
	"$ENV{SystemDrive}/WinDDK/*/inc/ddk"
)

IF(WDK_DIR)
	MESSAGE(STATUS "Found DDK in " ${DDK_DIR})

	INCLUDE_DIRECTORIES(
		${SALVIA_HOME_DIR}
		${SALVIA_BOOST_INCLUDE_DIR}
		${DDK_DIR}
		include
	)

	set (HEADER_FILES
		include/common.h
	)

	set (SOURCE_FILES
		src/adapter.cpp
		src/thunks.cpp
		
		src/d3d_sw_driver.def
	)

	ADD_LIBRARY( salvia_d3d_sw_driver SHARED
		${HEADER_FILES}	${SOURCE_FILES} )
	TARGET_LINK_LIBRARIES(salvia_d3d_sw_driver EFLIB)
	SET_TARGET_PROPERTIES(salvia_d3d_sw_driver PROPERTIES FOLDER "SALVIA Renderer")
	SALVIA_CONFIG_OUTPUT_PATHS(salvia_d3d_sw_driver)
ENDIF(WDK_DIR)
