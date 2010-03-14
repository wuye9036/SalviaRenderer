ADD_DEFINITIONS( -DWIN32
-D_LIB
-D_CRT_SECURE_NO_DEPRECATE
-D_SCL_SECURE_NO_DEPRECATE
-D_SECURE_SCL=0
-DBOOST_ALL_DYN_LINK
)
check_build_with_unicode()

set (HEADER_FILES
	include/config.h
	include/debug_helper.h
	include/eflib.h
	include/includer.h
	include/log_serializer.h
	include/math.h
	include/platform.h
	include/pool.h
	include/slog.h
	include/stl_utilities.h
	include/detail/streamx.h
	include/unicode_utility.h
	include/user_config.h
	include/util.h
	include/detail/win32.h
)

set (SOURCE_FILES
	src/collision_detection.cpp
	src/debug_helper.cpp
	src/initialize.cpp
	src/log_serializer.cpp
	src/math.cpp
	src/slog.cpp
	src/stringx.cpp
)

ADD_LIBRARY( EFLIB STATIC
${HEADER_FILES}
${SOURCE_FILES}
)