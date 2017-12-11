#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "movable_pointer.h"

struct T : public MovablePointee<T> {
  explicit T(int x_) : x(x_) {}
  int x;

  friend void swap(T& a, T& b) {
    a.swap(b);
    using std::swap;
    swap(a.x, b.x);
  }
};

TEST(MovablePointer, SwapPointee) {
  T a{1};
  T b{2};
  MovablePointer<T> ap(&a);
  MovablePointer<T> bp(&b);
  swap(a, b);
  EXPECT_EQ(2, a.x);
  EXPECT_EQ(1, b.x);
  EXPECT_EQ(1, ap->x);
  EXPECT_EQ(2, bp->x);
}
