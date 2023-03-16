cmake_minimum_required(VERSION 3.22)

project(CppLab)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Boost REQUIRED COMPONENTS thread)
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(unifex CONFIG REQUIRED)

# include(cmake/unifex/Findunifex.cmake)

function( sample_link_libraries target )
 target_link_libraries(${target} PRIVATE fmt::fmt-header-only)
 target_link_libraries(${target} PRIVATE range-v3 range-v3-meta range-v3::meta range-v3-concepts)
 target_link_libraries(${target} PRIVATE Boost::boost Boost::thread)
 target_link_libraries(${target} PRIVATE GTest::gtest GTest::gtest_main)
 target_link_libraries(${target} PRIVATE absl::strings)
endfunction()

file(GLOB sources20 CONFIGURE_DEPENDS *_Cpp20.cpp)
set_source_files_properties(${sources20} PROPERTIES
    COMPILE_FLAGS "/std:c++20"
)
# file(GLOB sources23 CONFIGURE_DEPENDS samples/*_Cpp23.cpp)
# add_library(samples23 SHARED ${sources23})
# target_compile_features(samples23 PUBLIC cxx_std_23)
# sample_link_libraries(samples23)

file(GLOB sources CONFIGURE_DEPENDS main.cpp)
add_executable(samples ${sources20})
target_compile_features(samples PUBLIC cxx_std_20)
sample_link_libraries(samples)
target_link_libraries(samples PRIVATE unifex::unifex)