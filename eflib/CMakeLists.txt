SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
)

set (HEADER_FILES
	include/common/initialize.h
	include/diagnostics/assert.h
	include/diagnostics/log.h
	include/diagnostics/log_serializer.h
	include/diagnostics/logrout.h
	include/io/stream.h
	include/math/math.h
	include/math/matrix.h
	include/math/quaternion.h
	include/math/vector.h
	include/memory/allocator.h
	include/memory/atomic.h
	include/memory/lifetime_manager.h
	include/memory/pool.h
	include/metaprog/enable_if.h
	include/metaprog/util.h
	include/platform/config.h
	include/platform/constant.h
	include/platform/disable_warnings.h
	include/platform/enable_warnings.h
	include/platform/boost_begin.h
	include/platform/boost_end.h
	include/platform/typedefs.h
	include/platform/win32.h
	include/platform/dl_loader.h
	include/string/string.h
	include/string/ustring.h
)

set (SOURCE_FILES
	src/collision_detection.cpp
	src/assert.cpp
	src/lifetime_manager.cpp
	src/initialize.cpp
	src/math.cpp
	src/quaternion.cpp
	src/string.cpp
	src/logrout.cpp
	src/cpuinfo.cpp
	src/dl_loader.cpp
)

ADD_LIBRARY( EFLIB STATIC
${HEADER_FILES}
${SOURCE_FILES}
)
SET_TARGET_PROPERTIES( EFLIB PROPERTIES FOLDER "Essential Lib")

SALVIA_CONFIG_OUTPUT_PATHS(EFLIB)