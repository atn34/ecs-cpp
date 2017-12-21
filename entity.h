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
    Entity(
        std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>*
            components,
        std::vector<MovablePointee<IndexTag>*>& entity_tmp)
        : components_(components), entity_tmp_(entity_tmp) {}

    template <typename T>
    T* get() {
      return entity_tmp_[Index<T, AllComponents...>::value] == nullptr
                 ? nullptr
                 : &reinterpret_cast<MovablePointee<WithIndexTag<T>>&>(
                        *entity_tmp_[Index<T, AllComponents...>::value])
                        ->component;
    }

    template <typename T>
    T& getOrAdd() {
      if (entity_tmp_[Index<T, AllComponents...>::value] != nullptr) {
        return reinterpret_cast<MovablePointee<WithIndexTag<T>>&>(
                   *entity_tmp_[Index<T, AllComponents...>::value])
            ->component;
      } else {
        auto& component_vec =
            std::get<Index<T, AllComponents...>::value>(*components_);
        component_vec.push_back({});
        auto& with_index_tag = component_vec.back();
        with_index_tag->component = {};
        with_index_tag->index = Index<T, AllComponents...>::value;
        auto* as_tag =
            reinterpret_cast<MovablePointee<IndexTag>*>(&with_index_tag);
        with_index_tag->next = as_tag;
        int prev_index = Index<T, AllComponents...>::value;
        while (entity_tmp_[prev_index] == nullptr) {
          prev_index = (prev_index - 1 + sizeof...(AllComponents)) %
                       sizeof...(AllComponents);
        }
        using std::swap;
        swap(with_index_tag->next, (*entity_tmp_[prev_index])->next);
        entity_tmp_[Index<T, AllComponents...>::value] = as_tag;
        return with_index_tag->component;
      }
    }

    template <typename T>
    void remove() {
      if (entity_tmp_[Index<T, AllComponents...>::value] == nullptr) {
        return;
      }
      auto& with_index_tag =
          *reinterpret_cast<MovablePointee<WithIndexTag<T>>*>(
              entity_tmp_[Index<T, AllComponents...>::value]);
      auto& component_vec =
          std::get<Index<T, AllComponents...>::value>(*components_);
      int prev_index = Index<T, AllComponents...>::value;
      do {
        prev_index = (prev_index - 1 + sizeof...(AllComponents)) %
                     sizeof...(AllComponents);
      } while (entity_tmp_[prev_index] == nullptr);
      using std::swap;
      swap(with_index_tag->next, (*entity_tmp_[prev_index])->next);
      swap(with_index_tag, component_vec.back());
      component_vec.pop_back();
      entity_tmp_[Index<T, AllComponents...>::value] = nullptr;
    }

    void removeAll() {
      auto it = std::find_if(
          entity_tmp_.begin(), entity_tmp_.end(),
          [](MovablePointee<IndexTag>* p) { return p != nullptr; });
      if (it == entity_tmp_.end()) {
        return;
      }
      auto first = *it;
      while (first != nullptr) {
        MovablePointer<IndexTag> next;
        using std::swap;
        swap(next, (*first)->next);
        remove_one(identity<sizeof...(AllComponents) - 1>{}, first);
        first = next.pointer();
      }
    }

   private:
    template <std::size_t t>
    struct identity {};

    void remove_one(identity<0>, MovablePointee<IndexTag>* p) {
      assert((*p)->index == 0);
      entity_tmp_[0] = nullptr;
      auto& component_vec = std::get<0>(*components_);
      auto& top = component_vec.back();
      using std::swap;
      swap(top, reinterpret_cast<decltype(top)>(*p));
      component_vec.pop_back();
    }

    template <std::size_t Index>
    void remove_one(identity<Index>, MovablePointee<IndexTag>* p) {
      if ((*p)->index == Index) {
        auto& component_vec = std::get<Index>(*components_);
        auto& top = component_vec.back();
        using std::swap;
        swap(top, reinterpret_cast<decltype(top)>(*p));
        component_vec.pop_back();
        entity_tmp_[Index] = nullptr;
      } else {
        remove_one(identity<Index - 1>{}, p);
      }
    }
    std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>*
        components_;
    std::vector<MovablePointee<IndexTag>*>& entity_tmp_;
  };

  template <typename Component, typename... Components, typename Lambda>
  void each(Lambda f) {
    auto& component_vec = std::get<index<Component>()>(components_);
    auto it = component_vec.begin();
    size_t last_size = component_vec.size();
    while (it != component_vec.end()) {
      entity_tmp_.clear();
      entity_tmp_.resize(sizeof...(AllComponents));
      auto& component = *it;
      bool matches = false;
      auto* first_in_query =
          reinterpret_cast<MovablePointee<IndexTag>*>(&component);
      each_helper<0, Component, Components...>(first_in_query, &matches);
      if (matches) {
        Entity entity(&components_, entity_tmp_);
        f(entity);
        if (component_vec.size() < last_size) {
          --last_size;
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

  template <std::size_t Dummy>
  void each_helper(MovablePointee<IndexTag>* p, bool* matches) {
    *matches = true;
    if (entity_tmp_[(*p)->index] != nullptr) {
      return;
    }
    entity_tmp_[(*p)->index] = p;
    each_helper<Dummy>((*p)->next.pointer(), matches);
  }
  template <std::size_t Dummy, typename Component, typename... Components>
  void each_helper(MovablePointee<IndexTag>* p, bool* matches) {
    if (entity_tmp_[(*p)->index] != nullptr) {
      return;
    }
    entity_tmp_[(*p)->index] = p;
    if ((*p)->index == index<Component>()) {
      each_helper<Dummy, Components...>((*p)->next.pointer(), matches);
    } else {
      each_helper<Dummy, Component, Components...>((*p)->next.pointer(),
                                                   matches);
    }
  }

  std::tuple<std::vector<MovablePointee<WithIndexTag<AllComponents>>>...>
      components_;
  std::vector<MovablePointee<IndexTag>*> entity_tmp_;
};

#endif /* ifndef ENTITY_H */
