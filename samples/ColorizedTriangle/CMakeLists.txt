SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_WTL_INCLUDE_PATH}
)
LINK_DIRECTORIES(
	${SALVIA_3RD_PARTY_LIBS}
	${SALVIA_BOOST_LIB_DIR}
	)

set (HEADER_FILES
	AboutDlg.h
	MainFrm.h
	resource.h
	ColorizedTriangle.h
	Timer.h
	stdafx.h
)

set (SOURCE_FILES
	ColorizedTriangle.cpp
	Timer.cpp
	stdafx.cpp
)

set (RESOURCE_FILES
	res/ColorizedTriangle.ico
	res/toolbar.bmp
	ColorizedTriangle.rc
)

ADD_EXECUTABLE(ColorizedTriangle
	${HEADER_FILES}
	${SOURCE_FILES}
	${RESOURCE_FILES}
)

TARGET_LINK_LIBRARIES(ColorizedTriangle
	EFLIB
	salviar
	salviax
	sasl_enums
	sasl_host
	sasl_syntaxtree
	sasl_semantic
	sasl_common
)

SALVIA_CONFIG_OUTPUT_PATHS(ColorizedTriangle)
SALVIA_CREATE_VCPROJ_USERFILE(ColorizedTriangle)