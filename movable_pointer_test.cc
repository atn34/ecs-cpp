#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "movable_pointer.h"

struct T {
  explicit T() : x(0) {}
  explicit T(int x_) : x(x_) {}
  int x;

  friend void swap(T& a, T& b) {
    using std::swap;
    swap(a.x, b.x);
  }
};

TEST(MovablePointer, SwapPointee) {
  MovablePointee<T> a{1};
  MovablePointee<T> b{2};
  MovablePointer<T> ap(&a);
  MovablePointer<T> bp(&b);
  swap(a, b);
  EXPECT_EQ(2, a.x);
  EXPECT_EQ(1, b.x);
  EXPECT_EQ(1, ap->x);
  EXPECT_EQ(2, bp->x);
}

TEST(MovablePointer, SwapPointer) {
  MovablePointee<T> a{1};
  MovablePointee<T> b{2};
  MovablePointer<T> ap(&a);
  MovablePointer<T> bp(&b);
  swap(ap, bp);
  EXPECT_EQ(1, a.x);
  EXPECT_EQ(2, b.x);
  EXPECT_EQ(2, ap->x);
  EXPECT_EQ(1, bp->x);
}

TEST(MovablePointer, SwapWithNull1) {
  MovablePointee<T> a{1};
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

TEST(MovablePointer, DestroyPointee) {
  MovablePointer<T> p;
  EXPECT_EQ(nullptr, p.get());
  {
    MovablePointee<T> a{1};
    p = &a;
    EXPECT_EQ(&a, p.get());
  }
  EXPECT_EQ(nullptr, p.get());
}

TEST(MovablePointer, ResizeVectorAndSwap) {
  std::vector<MovablePointee<T>> v;
  v.emplace_back(MovablePointee<T>{1});

  MovablePointer<T> p(&v[0]);

  v.resize(100);

  EXPECT_EQ(1, p->x);

  swap(v[0], v[1]);

  EXPECT_EQ(1, p->x);
}
