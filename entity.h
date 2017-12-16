#ifndef ENTITY_H
#define ENTITY_H

#include <tuple>

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
  template <typename... Components>
  void add_entity(Components... components) {
    add_entity_helper<0>(nullptr, components...);
  }

  template <typename Component>
  static constexpr std::size_t index() {
    return Index<Component, AllComponents...>::value;
  }

 private:
  struct IndexTag {
    size_t index;
    MovablePointer<IndexTag> next;
  };

  template <typename T>
  struct WithIndexTag : public IndexTag {
    T component;
  };

  template <std::size_t PreviousIndex, typename Component>
  MovablePointee<IndexTag>* add_entity_helper(MovablePointee<IndexTag>* prev,
                                              Component component) {
    static_assert(PreviousIndex < index<Component>() + 1,
                  "Components must always appear in order");
    auto& component_vec = std::get<index<Component>()>(components_);
    component_vec.push_back({});
    auto& with_index_tag = component_vec.back();
    with_index_tag->component = component;
    with_index_tag->index = index<Component>();
    auto* as_tag = reinterpret_cast<MovablePointee<IndexTag>*>(&with_index_tag);
    if (prev != nullptr) {
      prev->get()->next = as_tag;
    }
    return as_tag;
  }
  template <std::size_t PreviousIndex, typename Component,
            typename... Components>
  MovablePointee<IndexTag>* add_entity_helper(MovablePointee<IndexTag>* prev,
                                              Component component,
                                              Components... components) {
    MovablePointee<IndexTag>* as_tag =
        add_entity_helper<PreviousIndex>(prev, component);
    return add_entity_helper<index<Component>() + 1>(as_tag, components...);
  }

  std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>
      components_;
};

#endif /* ifndef ENTITY_H */
