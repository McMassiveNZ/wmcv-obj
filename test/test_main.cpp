#include <gtest/gtest.h>
#include <filesystem>

#include "wmcv_obj/wmcv_obj.h"

TEST(testmain, test_parse_simple_obj)
{
	std::filesystem::path path = DATA_DIR_PATH / std::filesystem::path("plane.obj");
	auto result = wmcv::ParseObj(path.make_preferred());

	EXPECT_EQ(1, result.subObjects.size());
	EXPECT_STREQ("Plane001", result.subObjects[0].name.c_str());
}
