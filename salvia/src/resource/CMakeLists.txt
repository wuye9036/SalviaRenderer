file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/salvia/resource/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(salvia_resource ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(salvia_resource PUBLIC ../../include)
target_link_libraries(salvia_resource eflib)
target_compile_features(salvia_resource PUBLIC cxx_std_20)

set_target_properties(salvia_resource PROPERTIES FOLDER "Salvia")
