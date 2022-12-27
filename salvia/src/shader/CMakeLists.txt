file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/salvia/shader/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(salvia_shader STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(salvia_shader PUBLIC ../../include)
target_link_libraries(salvia_shader PRIVATE eflib)
target_compile_features(salvia_shader PUBLIC cxx_std_20)

set_target_properties(salvia_shader PROPERTIES FOLDER "Salvia")
