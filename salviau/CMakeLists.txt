SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_WTL_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${Boost_LIBRARY_DIRS}
	${Freetype_LIBRARY_DIRS}
	${FreeImage_LIBRARY_DIRS}
)

set( APP_COMMON_HEADERS
	${SALVIA_HOME_DIR}/salviau/include/common/gui.h
	${SALVIA_HOME_DIR}/salviau/include/common/benchmark.h
	${SALVIA_HOME_DIR}/salviau/include/common/window.h
	${SALVIA_HOME_DIR}/salviau/include/common/timer.h
	${SALVIA_HOME_DIR}/salviau/include/common/sample_app.h
	${SALVIA_HOME_DIR}/salviau/include/common/path.h
)

set( APP_WIN_HEADERS
	${SALVIA_HOME_DIR}/salviau/include/win/win_gui.h
)

set( APP_WIN_SOURCES
	${SALVIA_HOME_DIR}/salviau/src/win/win_gui.cpp
)

set( APP_WIN_IMPLS
	${SALVIA_HOME_DIR}/salviau/src/win/targetver.h
	${SALVIA_HOME_DIR}/salviau/src/win/resource.h
	${SALVIA_HOME_DIR}/salviau/src/win/salvia_win_app.rc
)

set( APP_COMMON_SOURCES
	${SALVIA_HOME_DIR}/salviau/src/common/timer.cpp
	${SALVIA_HOME_DIR}/salviau/src/common/sample_app.cpp
	${SALVIA_HOME_DIR}/salviau/src/common/path.cpp
)

SOURCE_GROUP( "Header Files\\Common"	FILES ${APP_COMMON_HEADERS} )
SOURCE_GROUP( "Source Files\\Common"	FILES ${APP_COMMON_SOURCES} )

SOURCE_GROUP( "Header Files\\Win"					FILES ${APP_WIN_HEADERS} )
SOURCE_GROUP( "Source Files\\Win\\Sources"			FILES ${APP_WIN_SOURCES} )
SOURCE_GROUP( "Source Files\\Win\\Implementations"	FILES ${APP_WIN_IMPLS} )

if(MSVC)
ADD_LIBRARY( salviau SHARED
	${APP_COMMON_HEADERS}
	${APP_COMMON_SOURCES}

	${APP_WIN_HEADERS}
	${APP_WIN_SOURCES}
	${APP_WIN_IMPLS}
)
else(MSVC)
ADD_LIBRARY( salviau SHARED
	${APP_COMMON_HEADERS}
	${APP_COMMON_SOURCES}
)
endif(MSVC)

TARGET_LINK_LIBRARIES( salviau salviax EFLIB ${Boost_LIBRARIES}) 
SET_TARGET_PROPERTIES( salviau PROPERTIES FOLDER "Samples" )
install(TARGETS salviau)
install(
    CODE [[
  SET(EXCLUDE_PATTERNS
	"^libcdpl.*"
	"^.*linux.*"
	"^libGL.*"
	"^libX.*"
	"^libpython.*"
    "^.*system32.*"
    "^.*ms-win-core.*"
  )

  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES $<$<TARGET_EXISTS:salviau>:$<TARGET_FILE:salviau>>
    RESOLVED_DEPENDENCIES_VAR _r_deps
    UNRESOLVED_DEPENDENCIES_VAR _u_deps
    POST_EXCLUDE_REGEXES ${EXCLUDE_PATTERNS}
  )
  file(INSTALL ${_r_deps}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    FOLLOW_SYMLINK_CHAIN
  )
  message(STATUS "Resolved deps: ${_r_deps}")
  message(STATUS "Unresolved deps: ${_u_deps}")
]]
)
