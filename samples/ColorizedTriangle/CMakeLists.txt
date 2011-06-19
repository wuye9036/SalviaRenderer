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
	${SALVIA_LLVM_LIB_PATH}
	)

set( FRAMEWORK_HEADERS 
	Timer.h
	stdafx.h
	AboutDlg.h
	MainFrm.h
	resource.h
	ColorizedTriangle.h 
	)
	
SOURCE_GROUP( "Header Files\\Framework"
	FILES
	${FRAMEWORK_HEADERS}
)
	
set( VIEW_HEADERS ColorizedTriangleView.h )
SOURCE_GROUP( "Header Files\\View"
	FILES
	${VIEW_HEADERS}
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
	${FRAMEWORK_HEADERS}
	${VIEW_HEADERS}
	${SOURCE_FILES}
	${RESOURCE_FILES}
)

TARGET_LINK_LIBRARIES(ColorizedTriangle
	EFLIB
	salviar
	salviax
	sasl_enums
	sasl_host
	sasl_parser
	sasl_syntaxtree
	sasl_semantic
	sasl_common
	sasl_cgllvm
	${SASL_LLVM_LIBS}
)

SALVIA_CONFIG_OUTPUT_PATHS(ColorizedTriangle)
SALVIA_CREATE_VCPROJ_USERFILE(ColorizedTriangle)