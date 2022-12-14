set(HEADER_LIST 
  ../include/salvia/shader/constants.h
  ../include/salvia/shader/reflection.h
)

set(SOURCE_LIST
  dummy.cpp
)

add_library(shader STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(shader PUBLIC ../include)
target_compile_features(shader PUBLIC cxx_std_20)