#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "circular_interval.h"

TEST(CircularInterval, ContainsAndExpand) {
  CircularInterval interval{5, 2, 0};
  for (int i = 0; i < 5; ++i) {
    EXPECT_FALSE(interval.contains(i)) << i;
  }

  interval.expand(2);
  EXPECT_LE(interval.length, 5);
  EXPECT_TRUE(interval.contains(2));
  EXPECT_FALSE(interval.contains(1));
  EXPECT_FALSE(interval.contains(3));
  // Wrap around to the other side now, since expand always increases size (up
  // to modulus).
  interval.expand(2);
  EXPECT_EQ(interval.length, 5);

  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(interval.contains(i)) << i;
  }
}
