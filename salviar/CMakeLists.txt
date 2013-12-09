SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_BOOST_INCLUDE_DIR}
	${SALVIA_THREAD_POOL_INCLUDE_DIR}
)

SET( FOUNDATION_HEADERS
	include/salviar_forward.h
	include/decl.h
	include/enums.h
	include/colors.h
	include/colors_convertors.h
	include/lockfree_queue.h
	include/thread_pool.h
	include/binary_modules.h
	include/resource_data.h
	include/viewport.h
	include/voidptr.h
)

SET( PIPELINE_HEADERS
	include/stream_assembler.h
	include/clipper.h
	include/async_object.h
	include/geom_setup_engine.h
	include/index_fetcher.h
	include/input_layout.h
	include/rasterizer.h
	include/raster_state.h
	include/render_stages.h
	include/renderer.h
	include/renderer_impl.h
	include/renderer_capacity.h
	include/sync_renderer.h
	include/render_state.h
	include/vertex_cache.h
	include/framebuffer.h
	include/async_renderer.h
	include/render_core.h
	include/stream_state.h
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
	include/shader_cbuffer.h
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
	src/binary_modules.cpp
	src/resource_data.cpp
	src/buffer.cpp
)

set(BUFFER_SOURCES
	src/surface.cpp
	src/texture2d.cpp
	src/sampler.cpp
	src/texture_cube.cpp
)

set(PIPELINE_SOURCES
	src/clipper.cpp
	src/default_vertex_cache.cpp
	src/framebuffer.cpp
	src/geom_setup_engine.cpp
	src/rasterizer.cpp
	src/raster_state.cpp
	src/renderer.cpp
	src/sync_renderer.cpp
	src/resource_manager.cpp
	src/stream_assembler.cpp
	src/index_fetcher.cpp
	src/input_layout.cpp
	src/async_renderer.cpp
	src/render_core.cpp
	src/stream_state.cpp
	src/renderer_impl.cpp
	src/render_state.cpp
)

set(SHADER_SOURCES
	src/cpp_pixel_shader.cpp
	src/sampler_api.cpp
	src/shader.cpp
	src/shader_object.cpp
	src/shader_unit.cpp
	src/shader_cbuffer.cpp
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