SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_BOOST_INCLUDE_DIR}
	${SALVIA_HOME_DIR}
)

set (HEADER_FILES
	include/common/initialize.h
	include/diagnostics/assert.h
	include/diagnostics/log.h
	include/diagnostics/log_serializer.h
	include/diagnostics/logrout.h
	include/diagnostics/profiler.h
	include/io/stream.h
	include/math/math.h
	include/math/matrix.h
	include/math/quaternion.h
	include/math/vector.h
	include/memory/allocator.h
	include/memory/atomic.h
	include/memory/lifetime_manager.h
	include/memory/pool.h
	include/memory/bounded_buffer.h
	include/memory/vls.h
	include/utility/enable_if.h
	include/utility/unref_declarator.h
	include/utility/operator_bool.h
	include/utility/polymorphic_cast.h
	include/utility/scoped_value.h
	include/utility/shared_declaration.h
	include/utility/enum.h
	include/platform/config.h
	include/platform/constant.h
	include/platform/disable_warnings.h
	include/platform/enable_warnings.h
	include/platform/boost_begin.h
	include/platform/boost_end.h
	include/platform/typedefs.h
	include/platform/win32.h
	include/platform/dl_loader.h
	include/platform/intrin.h
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
	src/profiler.cpp
)

ADD_LIBRARY(EFLIB STATIC ${HEADER_FILES} ${SOURCE_FILES})

SALVIA_GROUP_BY_DIRECTORY( "Header Files" "include" HEADER_FILES )

SET_TARGET_PROPERTIES( EFLIB PROPERTIES FOLDER "SALVIA Renderer")

if(UNIX)
	TARGET_LINK_LIBRARIES(EFLIB dl) # This lib is for dlopen/dlsym/dlclose
endif()

SALVIA_CONFIG_OUTPUT_PATHS(EFLIB)
