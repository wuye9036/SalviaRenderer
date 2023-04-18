cmake_minimum_required(VERSION 3.22)

project(CppLab)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nameof CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

function( sample_link_libraries target )
 target_link_libraries(${target} PRIVATE fmt::fmt-header-only)
 target_link_libraries(${target} PRIVATE nameof::nameof)
 target_link_libraries(${target} PRIVATE GTest::gtest GTest::gtest_main)
 target_link_libraries(${target} PRIVATE range-v3 range-v3-meta range-v3::meta range-v3-concepts)
endfunction()

file(GLOB sources CONFIGURE_DEPENDS *.cpp *.h)
add_executable(samples ${sources})
target_compile_features(samples PUBLIC cxx_std_20)
sample_link_libraries(samples)