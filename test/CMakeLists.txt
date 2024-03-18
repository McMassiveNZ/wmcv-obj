include(gtest)

add_executable(
  wmcv-obj-test
  test_main.cpp
)

target_link_libraries(
  wmcv-obj-test
  wmcv-obj
  gtest_main
)

target_compile_definitions(
	wmcv-obj-test PRIVATE 
	DATA_DIR_PATH="${CMAKE_CURRENT_LIST_DIR}/data"
)

target_include_directories(
	wmcv-obj-test PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(wmcv-obj-test)
