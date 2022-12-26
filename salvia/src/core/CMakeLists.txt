file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/salvia/core/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(salvia_core STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(salvia_core PUBLIC ../../include)
target_link_libraries(salvia_core PRIVATE eflib)
target_compile_features(salvia_core PUBLIC cxx_std_20)

set_target_properties(salvia_core PROPERTIES FOLDER "Salvia")