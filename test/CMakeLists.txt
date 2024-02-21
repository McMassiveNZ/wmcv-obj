include(gtest)

add_executable(
  blank-slate-test
  test_main.cpp
)

target_link_libraries(
  blank-slate-test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(blank-slate-test)
