#ifndef ENTITY_H
#define ENTITY_H

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "movable_pointer.h"

// From
// https://stackoverflow.com/questions/26169198/how-to-get-the-index-of-a-type-in-a-variadic-type-pack
template <typename T, typename... Ts>
struct Index;
template <typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};
template <typename T, typename U, typename... Ts>
struct Index<T, U, Ts...>
    : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template <typename... AllComponents>
class World {
 public:
  template <typename Component>
  constexpr std::size_t index() {
    return Index<Component, AllComponents...>::value;
  }

 private:
  std::tuple<std::vector<MovablePointee<AllComponents>>...> components_;
};

#endif /* ifndef ENTITY_H */
