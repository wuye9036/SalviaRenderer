check_build_with_unicode()

INCLUDE_DIRECTORIES(
	${BOOST_HOME_DIR}
	${EFLIB_HOME_PATH}
)

set (HEADER_FILES
	include/common/initialize.h
	include/diagnostics/assert.h
	include/diagnostics/log.h
	include/diagnostics/log_serializer.h
	include/io/stream.h
	include/math/math.h
	include/math/matrix.h
	include/math/quaternion.h
	include/math/vector.h
	include/memory/allocator.h
	include/memory/lifetime_manager.h
	include/memory/pool.h
	include/metaprog/enable_if.h
	include/metaprog/util.h
	include/platform/config.h
	include/platform/constant.h
	include/platform/disable_warnings.h
	include/platform/enable_warnings.h
	include/platform/typedefs.h
	include/platform/win32.h
	include/string/string.h
)

set (SOURCE_FILES
	src/collision_detection.cpp
	src/assert.cpp
	src/lifetime_manager.cpp
	src/initialize.cpp
	src/log_serializer.cpp
	src/math.cpp
	src/quaternion.cpp
	src/log.cpp
	src/string.cpp
	src/memory.cpp
)

ADD_LIBRARY( EFLIB STATIC
${HEADER_FILES}
${SOURCE_FILES}
)

config_output_paths(EFLIB)