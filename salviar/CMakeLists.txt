SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
)
LINK_DIRECTORIES(${SALVIA_3RD_PARTY_LIBS})

SET( FOUNDATION_HEADERS
	include/salviar_forward.h
	include/decl.h
	include/enums.h
	include/colors.h
	include/colors_convertors.h
	include/lockfree_queue.h
	include/thread_pool.h
	include/binary_modules.h
)

SET( PIPELINE_HEADERS
	include/stream_assembler.h
	include/clipper.h
	include/index_fetcher.h
	include/input_layout.h
	include/presenter_dev.h
	include/rasterizer.h
	include/raster_state.h
	include/render_stage.h
	include/renderer.h
	include/renderer_capacity.h
	include/renderer_impl.h
	include/vertex_cache.h
	include/framebuffer.h
	include/async_renderer.h
)

set( SHADER_HEADERS
	include/host.h
	include/sampler_api.h
	include/shader.h
	include/shader_reflection.h
	include/shader_object.h
	include/shader_unit.h
	include/shader_utility.h
	include/shaderregs.h
	include/shaderregs_op.h
)

set( BUFFER_HEADERS
	include/buffer.h
	include/texture.h
	include/sampler.h
	include/surface.h
	include/resource_manager.h
)

SOURCE_GROUP( "Header Files\\Fundations" FILES
	${FOUNDATION_HEADERS}
)

SOURCE_GROUP( "Header Files\\Pipeline" FILES
	${PIPELINE_HEADERS}
)

SOURCE_GROUP( "Header Files\\Shader" FILES
	${SHADER_HEADERS}
)

SOURCE_GROUP( "Header Files\\Buffer" FILES
	${BUFFER_HEADERS}
)

set (HEADER_FILES
	${FOUNDATION_HEADERS}
	${PIPELINE_HEADERS}
	${SHADER_HEADERS}
	${BUFFER_HEADERS}
)

set(FOUNDATION_SOURCES
	src/thread_pool.cpp
	src/color_convertors.cpp
	src/result.cpp
	src/binary_modules.cpp
)

set(BUFFER_SOURCES
	src/surface.cpp
	src/texture1d.cpp
	src/texture2d.cpp
	src/sampler.cpp
	src/texture_cube.cpp
)

set(PIPELINE_SOURCES
	src/clipper.cpp
	src/default_vertex_cache.cpp
	src/framebuffer.cpp
	src/rasterizer.cpp
	src/raster_state.cpp
	src/renderer.cpp
	src/renderer_impl.cpp
	src/resource_manager.cpp
	src/stream_assembler.cpp
	src/index_fetcher.cpp
	src/input_layout.cpp
	src/async_renderer.cpp
)

set(SHADER_SOURCES
	src/host.cpp
	src/cpp_pixel_shader.cpp
	src/sampler_api.cpp
	src/shader.cpp
	src/shader_object.cpp
	src/shader_unit.cpp
)

SOURCE_GROUP( "Header Files" )
SOURCE_GROUP( "Source Files" )

SOURCE_GROUP( "Source Files\\Fundations" FILES
	${FOUNDATION_SOURCES}
)
SOURCE_GROUP( "Source Files\\Pipeline" FILES
	${PIPELINE_SOURCES}
)
SOURCE_GROUP( "Source Files\\Shader" FILES
	${SHADER_SOURCES}
)
SOURCE_GROUP( "Source Files\\Buffer" FILES
	${BUFFER_SOURCES}
)

set (SOURCE_FILES
	${FOUNDATION_SOURCES}
	${PIPELINE_SOURCES}
	${SHADER_SOURCES}
	${BUFFER_SOURCES}
)

ADD_LIBRARY( salviar STATIC
	${Buffers}
	${HEADER_FILES}
	${SOURCE_FILES}
)

TARGET_LINK_LIBRARIES(salviar EFLIB)
SET_TARGET_PROPERTIES( salviar PROPERTIES FOLDER "SALVIA Renderer" )

SALVIA_CONFIG_OUTPUT_PATHS(salviar)