
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"

struct A {};
struct B {};

TEST(Templates, TypeIndex) {
  World<A, B> world;
  EXPECT_EQ(0, world.index<A>());
  EXPECT_EQ(1, world.index<B>());

  world.add_entity(A{});
  world.add_entity(A{}, B{});
  world.add_entity(B{});
}

TEST(World, AddEntity) {
  World<A, B> world;
  EXPECT_EQ(0, world.index<A>());
  EXPECT_EQ(1, world.index<B>());

  world.add_entity(A{});
  world.add_entity(A{}, B{});
  world.add_entity(B{});
}