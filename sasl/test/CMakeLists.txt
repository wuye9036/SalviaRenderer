set(SOURCE_LIST "")

foreach(comp_name "main" "common" "parser" "drivers")
  file(GLOB_RECURSE TMP_SOURCE_LIST CONFIGURE_DEPENDS "./${comp_name}/*.cpp")
  list(APPEND SOURCE_LIST ${TMP_SOURCE_LIST})
endforeach()

add_executable(sasl_test ${SOURCE_LIST})
target_link_libraries(sasl_test PUBLIC sasl_drivers sasl_common eflib GTest::gtest GTest::gtest_main Boost::boost Boost::program_options $<$<PLATFORM_ID:UNIX>:dl>)
target_compile_features(sasl_test PUBLIC cxx_std_20)
deploy_dlls(sasl_test)