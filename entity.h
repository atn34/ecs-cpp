#ifndef ENTITY_H
#define ENTITY_H

#include <tuple>
#include <vector>

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
  World() : entity_tmp_(sizeof...(AllComponents)) {}

  template <typename Component, typename... Components>
  void add_entity(Component component, Components... components) {
    auto* first = add_entity_helper<0>(nullptr, component);
    auto* last = add_entity_helper<index<Component>()>(first, components...);
    last->get()->next = first;
  }

  class Entity {
   public:
    Entity(std::vector<MovablePointee<IndexTag>*>& entity_tmp)
        : entity_tmp_(entity_tmp) {}

    template <typename T>
    T& get() {
      return reinterpret_cast<MovablePointee<WithIndexTag<T>>&>(
                 *entity_tmp_[Index<T, AllComponents...>::value])
          ->component;
    }

    void remove() { remove_ = true; }
    bool remove_ = false;

   private:
    std::vector<MovablePointee<IndexTag>*>& entity_tmp_;
  };

  template <typename Component, typename... Components, typename Lambda>
  void each(Lambda f) {
    auto& component_vec = std::get<index<Component>()>(components_);
    auto it = component_vec.begin();
    while (it != component_vec.end()) {
      entity_tmp_.clear();
      entity_tmp_.resize(sizeof...(AllComponents));
      auto& component = *it;
      if (each_helper<1, Component, Components...>(
              reinterpret_cast<MovablePointee<IndexTag>*>(&component))) {
        Entity entity(entity_tmp_);
        f(entity);
        if (entity.remove_) {
          remove(reinterpret_cast<MovablePointee<IndexTag>*>(
              entity_tmp_[index<Component>()]));
          continue;
        }
      }
      ++it;
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

  template <std::size_t TupleIndex>
  bool each_helper(MovablePointee<IndexTag>*) {
    return true;
  }
  template <std::size_t TupleIndex, typename Component, typename... Components>
  bool each_helper(MovablePointee<IndexTag>* p) {
    if ((*p)->index != index<Component>()) {
      return false;
    }
    entity_tmp_[index<Component>()] = p;
    return each_helper<TupleIndex + 1, Components...>((*p)->next.pointer());
  }

  std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>
      components_;
  std::vector<MovablePointee<IndexTag>*> entity_tmp_;
};

#endif /* ifndef ENTITY_H */
