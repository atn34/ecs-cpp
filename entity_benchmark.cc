#include "entity.h"

#include <benchmark/benchmark.h>

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};
struct G {};
struct H {};
struct I {};
struct J {};

static void BM_IterateOneComponentAtATime(benchmark::State& state) {
  typedef World<A, B, C, D, E, F, G, H, I, J> World;
  World world;
  for (int i = 0; i < state.range(0); ++i) {
    world.add_entity(A{}, B{}, C{}, D{}, E{}, F{}, G{}, H{}, I{}, J{});
  }
  for (auto _ : state) {
    world.each<A>([](World::Entity&) {});
    world.each<B>([](World::Entity&) {});
    world.each<C>([](World::Entity&) {});
    world.each<D>([](World::Entity&) {});
    world.each<E>([](World::Entity&) {});
    world.each<F>([](World::Entity&) {});
    world.each<G>([](World::Entity&) {});
    world.each<H>([](World::Entity&) {});
    world.each<I>([](World::Entity&) {});
    world.each<J>([](World::Entity&) {});
  }
}

static void BM_RemoveWithDistantPrev(benchmark::State& state) {
  typedef World<A, B, C, D, E, F, G, H, I, J> World;
  World world;
  for (int i = 0; i < state.range(0); ++i) {
    world.add_entity(A{}, J{});
  }
  for (auto _ : state) {
    world.each<J>([](World::Entity& e) { e.remove<J>(); });
    world.each<A>([](World::Entity& e) { e.getOrAdd<J>(); });
  }
}

BENCHMARK(BM_IterateOneComponentAtATime)->Range(8, 8 << 10);
BENCHMARK(BM_RemoveWithDistantPrev)->Range(8, 8 << 10);

BENCHMARK_MAIN();
