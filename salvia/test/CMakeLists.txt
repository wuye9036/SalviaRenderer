file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_executable(salvia_test ${SOURCE_LIST})
target_include_directories(salvia_test PUBLIC ../include)
target_compile_features(salvia_test PUBLIC cxx_std_20)
target_link_libraries(salvia_test PRIVATE salvia_core GTest::gtest GTest::gtest_main Boost::boost $<$<PLATFORM_ID:UNIX>:dl> eflib)

set_target_properties(salvia_test PROPERTIES FOLDER "Salvia")
