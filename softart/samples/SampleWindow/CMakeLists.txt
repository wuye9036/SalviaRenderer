ADD_DEFINITIONS( -DWIN32
-D_LIB
-D_CRT_SECURE_NO_DEPRECATE
-D_SCL_SECURE_NO_DEPRECATE
-D_SECURE_SCL=0
-DBOOST_ALL_DYN_LINK
)
check_build_with_unicode()

INCLUDE_DIRECTORIES(../../)

set (HEADER_FILES
	AboutDlg.h
	MainFrm.h
	resource.h
	SRSampleWindow.h
	SRSampleWindowView.h
	stdafx.h
)

set (SOURCE_FILES
	SRSampleWindow.cpp
	stdafx.cpp
)

set (SOURCE_FILES
	SRSampleWindow.cpp
	stdafx.cpp
)

set (RESOURCE_FILES
	res/SRSampleWindow.ico
	res/toolbar.bmp
	SRSampleWindow.rc
)

ADD_EXECUTABLE(SampleWindow
${HEADER_FILES}
${SOURCE_FILES}
${RESOURCE_FILES}
)

TARGET_LINK_LIBRARIES(SampleWindow
	EFLIB
	SoftArt
	SoftArtX
)
config_output_paths(SampleWindow)


ogre_create_vcproj_userfile(SampleWindow)