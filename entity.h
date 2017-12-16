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
 private:
  template <typename... Components>
  friend class Entity;

  struct IndexTag {
    size_t index;
    MovablePointer<IndexTag> next;
  };

  template <typename T>
  struct WithIndexTag : public IndexTag {
    T component;
  };

 public:
  template <typename Component, typename... Components>
  void add_entity(Component component, Components... components) {
    auto* first = add_entity_helper<0>(nullptr, component);
    auto* last = add_entity_helper<index<Component>()>(first, components...);
    last->get()->next = first;
  }

  template <typename... Components>
  class Entity {
   public:
    Entity(World<AllComponents...>* world,
           std::tuple<MovablePointee<WithIndexTag<Components>>*...>* components)
        : this_(world), components_(components) {}

    template <typename T>
    T& get() {
      return std::get<Index<T, Components...>::value>(*components_)
          ->get()
          ->component;
    }

    void remove() {
      this_->remove(reinterpret_cast<MovablePointee<IndexTag>*>(
          std::get<0>(*components_)));
    }

   private:
    World<AllComponents...>* this_;
    std::tuple<MovablePointee<WithIndexTag<Components>>*...>* components_;
  };

  template <typename Component, typename... Components, typename Lambda>
  void each(Lambda f) {
    for (auto& component : std::get<index<Component>()>(components_)) {
      std::tuple<MovablePointee<WithIndexTag<Component>>*,
                 MovablePointee<WithIndexTag<Components>>*...>
          components;
      std::get<0>(components) = &component;
      if (each_helper<decltype(components), Components...>(components,
                                                           &component->next)) {
        Entity<Component, Components...> entity(this, &components);
        f(entity);
      }
    }
  }

  template <typename Component>
  static constexpr std::size_t index() {
    return Index<Component, AllComponents...>::value;
  }

 private:
  template <std::size_t t>
  struct identity {};

  void remove(MovablePointee<IndexTag>* first) {
    while (first != nullptr) {
      MovablePointer<IndexTag> next;
      using std::swap;
      swap(next, (*first)->next);
      remove_helper(identity<sizeof...(AllComponents) - 1>{}, first);
      first = next.pointer();
    }
  }

  void remove_helper(identity<0>, MovablePointee<IndexTag>* p) {
    assert((*p)->index == 0);
    auto& component_vec = std::get<0>(components_);
    auto& top = component_vec.back();
    using std::swap;
    swap(top, reinterpret_cast<decltype(top)>(*p));
    component_vec.pop_back();
  }

  template <std::size_t Index>
  void remove_helper(identity<Index>, MovablePointee<IndexTag>* p) {
    if ((*p)->index == Index) {
      auto& component_vec = std::get<Index>(components_);
      auto& top = component_vec.back();
      using std::swap;
      swap(top, reinterpret_cast<decltype(top)>(*p));
      component_vec.pop_back();
    } else {
      remove_helper(identity<Index - 1>{}, p);
    }
  }

  template <std::size_t PreviousIndex>
  MovablePointee<IndexTag>* add_entity_helper(MovablePointee<IndexTag>* prev) {
    return prev;
  }
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

  template <typename Entity>
  bool each_helper(Entity&, MovablePointer<IndexTag>*) {
    return true;
  }
  template <typename Entity, typename Component, typename... Components>
  bool each_helper(Entity& e, MovablePointer<IndexTag>* p) {
    if ((*p)->index != index<Component>()) {
      return false;
    }
    std::get<index<Component>()>(e) =
        reinterpret_cast<MovablePointee<WithIndexTag<Component>>*>(p);
    return each_helper(e, &(*p)->next);
  }

  std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>
      components_;
};

#endif /* ifndef ENTITY_H */
