#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "movable_pointer.h"

struct T : public MovablePointee<T> {
  explicit T() : x(0) {}
  explicit T(int x_) : x(x_) {}
  int x;

  friend void swap(T& a, T& b) {
    a.swap(b);
    using std::swap;
    swap(a.x, b.x);
  }

  T(T&& other) noexcept {
    swap(other);
    using std::swap;
    swap(x, other.x);
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

TEST(MovablePointer, SwapPointer) {
  T a{1};
  T b{2};
  MovablePointer<T> ap(&a);
  MovablePointer<T> bp(&b);
  swap(ap, bp);
  EXPECT_EQ(1, a.x);
  EXPECT_EQ(2, b.x);
  EXPECT_EQ(2, ap->x);
  EXPECT_EQ(1, bp->x);
}

TEST(MovablePointer, SwapWithNull1) {
  T a{1};
  MovablePointer<T> ap(&a);
  MovablePointer<T> bp;
  swap(ap, bp);
  EXPECT_EQ(nullptr, ap.get());
  EXPECT_EQ(1, bp->x);
}

TEST(MovablePointer, SwapNullNull) {
  MovablePointer<T> ap;
  MovablePointer<T> bp;
  swap(ap, bp);
  EXPECT_EQ(nullptr, ap.get());
  EXPECT_EQ(nullptr, bp.get());
}

TEST(MovablePointee, ResizeVector) {
  std::vector<T> v;
  v.emplace_back(T{1});

  MovablePointer<T> p(&v[0]);

  v.resize(100);

  EXPECT_EQ(1, p->x);
}
