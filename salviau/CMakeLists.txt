SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_WTL_INCLUDE_PATH}
)

LINK_DIRECTORIES(
	${SALVIA_BOOST_LIB_DIR}
)

set( APP_COMMON_HEADERS
	${SALVIA_HOME_DIR}/salviau/include/common/application.h
	${SALVIA_HOME_DIR}/salviau/include/common/presenter_utility.h
	${SALVIA_HOME_DIR}/salviau/include/common/window.h
	${SALVIA_HOME_DIR}/salviau/include/common/timer.h
)

set( APP_WTL_HEADERS
	${SALVIA_HOME_DIR}/salviau/include/wtl/wtl_application.h
)

set( APP_WTL_SOURCES
	${SALVIA_HOME_DIR}/salviau/src/wtl/wtl_application.cpp
)

set( APP_WTL_IMPLS
	${SALVIA_HOME_DIR}/salviau/src/wtl/AboutDlg.h
	${SALVIA_HOME_DIR}/salviau/src/wtl/MainFrm.h
	${SALVIA_HOME_DIR}/salviau/src/wtl/resource.h
	${SALVIA_HOME_DIR}/salviau/src/wtl/stdafx.cpp
	${SALVIA_HOME_DIR}/salviau/src/wtl/stdafx.h
	${SALVIA_HOME_DIR}/salviau/src/wtl/WTLAppFrame.h
	${SALVIA_HOME_DIR}/salviau/src/wtl/WTLAppFrame.rc
	${SALVIA_HOME_DIR}/salviau/src/wtl/WTLAppFrameView.h
)


set( APP_COMMON_SOURCES
	${SALVIA_HOME_DIR}/salviau/src/common/timer.cpp
	${SALVIA_HOME_DIR}/salviau/src/common/presenter_utility.cpp
	${SALVIA_HOME_DIR}/salviau/src/common/application.cpp
)

SOURCE_GROUP( "Header Files\\Common"	FILES ${APP_COMMON_HEADERS} )
SOURCE_GROUP( "Source Files\\Common"	FILES ${APP_COMMON_SOURCES} )

SOURCE_GROUP( "Header Files\\WTL"					FILES ${APP_WTL_HEADERS} )
SOURCE_GROUP( "Source Files\\WTL\\Sources"			FILES ${APP_WTL_SOURCES} )
SOURCE_GROUP( "Source Files\\WTL\\Implementations"	FILES ${APP_WTL_IMPLS} )

ADD_LIBRARY( salviau SHARED
	${APP_COMMON_HEADERS}
	${APP_COMMON_SOURCES}
	
	${APP_WTL_HEADERS}
	${APP_WTL_SOURCES}
	${APP_WTL_IMPLS}
)

TARGET_LINK_LIBRARIES( salviau EFLIB ) 
SET_TARGET_PROPERTIES( salviau PROPERTIES FOLDER "Samples" )

SALVIA_CONFIG_OUTPUT_PATHS(salviau)
