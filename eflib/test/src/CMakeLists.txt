file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_executable(eflib_test ${SOURCE_LIST})
target_include_directories(eflib_test PUBLIC ../include)
target_compile_features(eflib_test PUBLIC cxx_std_20)
target_link_libraries(eflib_test PRIVATE GTest::gtest GTest::gtest_main Boost::boost $<$<PLATFORM_ID:UNIX>:dl> eflib)

set_target_properties(eflib_test PROPERTIES FOLDER "SALVIA Renderer")
