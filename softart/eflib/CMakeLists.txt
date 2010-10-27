check_build_with_unicode()

INCLUDE_DIRECTORIES(
	${BOOST_HOME_DIR}
)

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
	include/detail/collision_detection.h
	include/detail/constant.h
	include/detail/initialize.h
	include/detail/matrix.h
	include/detail/memory.h
	include/detail/quaternion.h
	include/detail/streamx.h
	include/detail/stringx.h
	include/detail/swizzle.h
	include/detail/typedefs.h
	include/detail/vector.h
	include/detail/win32.h
	include/detail/write_mask.h
)

set (SOURCE_FILES
	src/collision_detection.cpp
	src/debug_helper.cpp
	src/initialize.cpp
	src/log_serializer.cpp
	src/math.cpp
	src/quaternion.cpp
	src/slog.cpp
	src/stringx.cpp
	src/memory.cpp
)

ADD_LIBRARY( EFLIB STATIC
${HEADER_FILES}
${SOURCE_FILES}
)

config_output_paths(EFLIB)