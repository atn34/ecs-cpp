#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"

struct A {};
struct B {};

TEST(Templates, TypeIndex) {
  World<A, B> world;
  EXPECT_EQ(0, world.index<A>());
  EXPECT_EQ(1, world.index<B>());
}

TEST(World, AddEntity) {
  World<A, B> world;

  world.add_entity(A{});
  world.add_entity(A{}, B{});
  world.add_entity(B{});
}

TEST(World, Each) {
  typedef World<A, B> World;
  World world;

  world.add_entity(A{});
  world.add_entity(A{}, B{});
  world.add_entity(B{});

  {
    int count = 0;
    auto lambda = [&count](World::Entity<A, B>&) { ++count; };
    world.each<A, B>(lambda);
    EXPECT_EQ(1, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<A>&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<B>&) { ++count; };
    world.each<B>(lambda);
    EXPECT_EQ(2, count);
  }
}

struct C {
  int c = 0;
};

TEST(World, EachWithState) {
  typedef World<C> World;
  World world;

  world.add_entity(C{});
  world.add_entity(C{});
  world.add_entity(C{});

  int count = 0;
  auto lambda = [&count](World::Entity<C>& entity) {
    int& c = entity.get<C>().c;
    EXPECT_EQ(count, c);
    ++c;
  };
  world.each<C>(lambda);
  ++count;
  world.each<C>(lambda);
  ++count;
  world.each<C>(lambda);
}

TEST(World, RemoveEntity) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(C{});
  world.add_entity(C{});
  world.add_entity(A{}, C{});
  world.add_entity(A{}, B{}, C{});
  world.add_entity(B{}, C{});

  {
    int count = 0;
    auto lambda = [&count](World::Entity<C>&) { ++count; };
    world.each<C>(lambda);
    EXPECT_EQ(5, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<A>&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<B>&) { ++count; };
    world.each<B>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    auto lambda = [](World::Entity<B, C>& entity) { entity.remove(); };
    world.each<B, C>(lambda);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<C>&) { ++count; };
    world.each<C>(lambda);
    EXPECT_EQ(3, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity<A>&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(1, count);
  }
}

TEST(World, AddEntityWhileIterating) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(C{});

  world.each<C>([&](World::Entity<C>&) { world.add_entity(A{}, B{}); });
  int count = 0;
  world.each<A, B>([&](World::Entity<A, B>&) { ++count; });
  EXPECT_EQ(1, count);
}
