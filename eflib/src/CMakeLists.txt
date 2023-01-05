file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "../include/**/*.h")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/../include/eflib" PREFIX "Header Files" FILES ${HEADER_LIST})

add_library(eflib STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(eflib PUBLIC ../include)
target_link_libraries(eflib PRIVATE Boost::boost $<$<PLATFORM_ID:UNIX>:dl>)
target_compile_features(eflib PUBLIC cxx_std_20)
