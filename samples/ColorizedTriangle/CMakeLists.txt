SALVIA_CHECK_BUILD_WITH_UNICODE()

SET( FRAMEWORK_HEADERS 
	Timer.h 	stdafx.h
	AboutDlg.h	MainFrm.h
	resource.h	ColorizedTriangle.h  )
SOURCE_GROUP( "Header Files\\Framework" FILES ${FRAMEWORK_HEADERS} )
	
SET( VIEW_HEADERS ColorizedTriangleView.h )
SOURCE_GROUP( "Header Files\\View" FILES ${VIEW_HEADERS} )

SET(SOURCE_FILES ColorizedTriangle.cpp Timer.cpp stdafx.cpp )
SET(RESOURCE_FILES res/ColorizedTriangle.ico res/toolbar.bmp ColorizedTriangle.rc )
SET(ADDITIONAL_FILES ${FRAMEWORK_HEADERS} ${VIEW_HEADERS} )

SALVIA_CONFIG_SAMPLE_PROJECT( ColorizedTriangle )