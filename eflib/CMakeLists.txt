file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "./include/**/*.h")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "./src/*.cpp")

add_library(eflib STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(eflib PUBLIC ./include)
target_link_libraries(eflib PRIVATE Boost::boost $<$<PLATFORM_ID:UNIX>:dl>)

SALVIA_GROUP_BY_DIRECTORY( "Header Files" "eflib" HEADER_FILES )
SET_TARGET_PROPERTIES(eflib PROPERTIES FOLDER "SALVIA Renderer")


