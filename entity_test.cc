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

TEST(World, Empty) {
  typedef World<A> World;
  World world;
  int count = 0;
  auto lambda = [&count](World::Entity&) { ++count; };
  world.each<A>(lambda);
  EXPECT_EQ(0, count);
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
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A, B>(lambda);
    EXPECT_EQ(1, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
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
  auto lambda = [&count](World::Entity& entity) {
    int& c = entity.get<C>()->c;
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
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<C>(lambda);
    EXPECT_EQ(5, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<B>(lambda);
    EXPECT_EQ(2, count);
  }
  {
    auto lambda = [](World::Entity& entity) { entity.removeAll(); };
    world.each<B, C>(lambda);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<C>(lambda);
    EXPECT_EQ(3, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(1, count);
  }
}

TEST(World, AddEntityWhileIterating) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(C{});

  world.each<C>([&](World::Entity&) { world.add_entity(A{}, B{}); });
  int count = 0;
  world.each<A, B>([&](World::Entity&) { ++count; });
  EXPECT_EQ(1, count);
}

struct P {
  int x = 0;
};

struct V {
  int x = 0;
};

TEST(World, AsanRepro) {
  typedef World<P, V> World;
  World world;
  world.add_entity(P{}, V{});
  world.each<P, V>([](World::Entity& e) {
    auto* p = e.get<P>();
    auto* v = e.get<V>();
    if (p->x == v->x) {
    }
  });
}

TEST(World, GetOrAdd) {
  typedef World<A, B> World;
  World world;

  world.add_entity(B{});

  world.each<B>([](World::Entity& e) { e.getOrAdd<A>(); });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A, B>(lambda);
    EXPECT_EQ(1, count);
  }
}

TEST(World, GetOrAdd2) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(A{});

  world.each<A>([](World::Entity& e) { e.getOrAdd<C>(); });
  world.each<C>([](World::Entity& e) { e.getOrAdd<B>(); });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A, B, C>(lambda);
    EXPECT_EQ(1, count);
  }
}

TEST(World, Remove) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(A{}, B{}, C{});

  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A, B, C>(lambda);
    EXPECT_EQ(1, count);
  }
  world.each<A, B, C>([](World::Entity& e) { e.remove<C>(); });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A, B, C>(lambda);
    EXPECT_EQ(0, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<B>(lambda);
    EXPECT_EQ(1, count);
  }
  world.each<A>([](World::Entity& e) { e.remove<B>(); });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<B>(lambda);
    EXPECT_EQ(0, count);
  }
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(1, count);
  }
  world.each<A>([](World::Entity& e) { e.remove<A>(); });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(0, count);
  }
}

TEST(World, GetAndRemove) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(A{});

  world.each<A>([](World::Entity& e) {
    e.getOrAdd<B>();
    e.remove<A>();
  });
  world.each<B>([](World::Entity& e) {
    e.getOrAdd<C>();
    e.remove<B>();
  });
  world.each<C>([](World::Entity& e) {
    e.getOrAdd<A>();
    e.remove<C>();
  });
  {
    int count = 0;
    auto lambda = [&count](World::Entity&) { ++count; };
    world.each<A>(lambda);
    EXPECT_EQ(1, count);
  }
}

TEST(World, CornerCases) {
  typedef World<A, B, C> World;
  World world;

  world.add_entity(A{}, B{}, C{});
  world.each<A, C>([](World::Entity& e) {
    e.getOrAdd<A>();
    e.remove<A>();
    e.remove<A>();
    e.removeAll();
  });
}

TEST(World, DoubleRemoveAll) {
  typedef World<A> World;
  World world;

  world.add_entity(A{});
  world.each<A>([](World::Entity& e) {
    e.removeAll();
    e.removeAll();
  });
}
