#include <co.hpp>

#include <gtest/gtest.h>

TEST(c_version_0, packs_components)
{
	co_version_0 version = CO_VERSION_0(1, 2, 3, 4);
	EXPECT_EQ(CO_VERSION_0_EPOCH(version), 1);
	EXPECT_EQ(CO_VERSION_0_MAJOR(version), 2);
	EXPECT_EQ(CO_VERSION_0_MINOR(version), 3);
	EXPECT_EQ(CO_VERSION_0_PATCH(version), 4);
	EXPECT_LT(CO_VERSION_0(0, 1, 9, 9), CO_VERSION_0(0, 2, 0, 0));
}
