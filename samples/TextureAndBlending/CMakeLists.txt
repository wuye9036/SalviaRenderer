SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
${SALVIA_HOME_DIR}
${SALVIA_3RD_PARTY_INCLUDES}
)

LINK_DIRECTORIES(${SALVIA_3RD_PARTY_LIBS})

set (HEADER_FILES
	AboutDlg.h
	MainFrm.h
	resource.h
	SRSampleWindow.h
	SRSampleWindowView.h
	Timer.h
	stdafx.h
)

set (SOURCE_FILES
	SRSampleWindow.cpp
	Timer.cpp
	stdafx.cpp
)

set (RESOURCE_FILES
	res/SRSampleWindow.ico
	res/toolbar.bmp
	SRSampleWindow.rc
)

ADD_EXECUTABLE(TextureAndBlending
${HEADER_FILES}
${SOURCE_FILES}
${RESOURCE_FILES}
)

TARGET_LINK_LIBRARIES(TextureAndBlending
	EFLIB
	salviar
	salviax
)

SALVIA_CONFIG_OUTPUT_PATHS(TextureAndBlending)
SALVIA_CREATE_VCPROJ_USERFILE(TextureAndBlending)