#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "movable_pointer.h"

struct A {
  explicit A() : x(0) {}
  explicit A(int x_) : x(x_) {}
  int x;

  friend void swap(A& a, A& b) {
    using std::swap;
    swap(a.x, b.x);
  }
};

TEST(MovablePointer, SwapPointee) {
  MovablePointee<A> a{1};
  MovablePointee<A> b{2};
  MovablePointer<A> ap(&a);
  MovablePointer<A> bp(&b);
  swap(a, b);
  EXPECT_EQ(2, a->x);
  EXPECT_EQ(1, b->x);
  EXPECT_EQ(1, ap->x);
  EXPECT_EQ(2, bp->x);
}

TEST(MovablePointer, SwapPointer) {
  MovablePointee<A> a{1};
  MovablePointee<A> b{2};
  MovablePointer<A> ap(&a);
  MovablePointer<A> bp(&b);
  swap(ap, bp);
  EXPECT_EQ(1, a->x);
  EXPECT_EQ(2, b->x);
  EXPECT_EQ(2, ap->x);
  EXPECT_EQ(1, bp->x);
}

TEST(MovablePointer, SwapWithNull1) {
  MovablePointee<A> a{1};
  MovablePointer<A> ap(&a);
  MovablePointer<A> bp;
  swap(ap, bp);
  EXPECT_EQ(nullptr, ap.get());
  EXPECT_EQ(1, bp->x);
}

TEST(MovablePointer, SwapNullNull) {
  MovablePointer<A> ap;
  MovablePointer<A> bp;
  swap(ap, bp);
  EXPECT_EQ(nullptr, ap.get());
  EXPECT_EQ(nullptr, bp.get());
}

TEST(MovablePointer, DestroyPointee) {
  MovablePointer<A> p;
  EXPECT_EQ(nullptr, p.get());
  {
    MovablePointee<A> a{1};
    p = &a;
    EXPECT_EQ(a.get(), p.get());
  }
  EXPECT_EQ(nullptr, p.get());
}

TEST(MovablePointer, ResizeVectorAndSwap) {
  std::vector<MovablePointee<A>> v;
  v.emplace_back(MovablePointee<A>{1});

  MovablePointer<A> p(&v[0]);

  v.resize(100);

  EXPECT_EQ(1, p->x);

  swap(v[0], v[1]);

  EXPECT_EQ(1, p->x);
}
