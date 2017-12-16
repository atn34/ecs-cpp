#ifndef MOVABLE_POINTER_H
#define MOVABLE_POINTER_H

#include <algorithm>
#include <cassert>

template <typename T>
class MovablePointer;
template <typename T>
class MovablePointee;

template <typename T>
void swap(MovablePointer<T>& a, MovablePointer<T>& b) {
  using std::swap;
  swap(a.t_, b.t_);
  if (a.t_ != nullptr) {
    a.t_->return_address_ = &a;
  }
  if (b.t_ != nullptr) {
    b.t_->return_address_ = &b;
  }
}

template <typename T>
void swap(MovablePointee<T>& a, MovablePointee<T>& b) {
  using std::swap;
  swap(a.pointee_, b.pointee_);
  swap(a.return_address_, b.return_address_);
  if (a.return_address_ != nullptr) {
    a.return_address_->t_ = &a;
  }
  if (b.return_address_ != nullptr) {
    b.return_address_->t_ = &b;
  }
}

template <typename T>
class MovablePointee final {
 public:
  MovablePointee() : return_address_(nullptr), pointee_() {}
  template <typename... Args>
  MovablePointee(Args&&... args)
      : return_address_(nullptr), pointee_(std::forward<Args&&...>(args...)) {}

  MovablePointee(const MovablePointee<T>&) = delete;
  MovablePointee<T>& operator=(const MovablePointee<T>&) = delete;

  MovablePointee(MovablePointee<T>&& other) noexcept : MovablePointee() {
    swap(*this, other);
  }

  MovablePointee<T>& operator=(MovablePointee<T>&& other) {
    swap(*this, other);
    return *this;
  }

  ~MovablePointee() {
    if (return_address_ != nullptr) {
      return_address_->t_ = nullptr;
    }
  }

  T& operator*() { return *get(); }
  T* operator->() { return get(); }
  T* get() { return &pointee_; }

 private:
  MovablePointer<T>* return_address_;
  T pointee_;

  friend class MovablePointer<T>;
  friend void swap<>(MovablePointer<T>& a, MovablePointer<T>& b);
  friend void swap<>(MovablePointee<T>& a, MovablePointee<T>& b);
};

template <typename T>
class MovablePointer final {
 public:
  MovablePointer() : t_(nullptr) {}
  explicit MovablePointer(MovablePointee<T>* t) : MovablePointer() {
    *this = t;
  }
  MovablePointer<T>& operator=(MovablePointee<T>* t) {
    t_ = t;
    if (t_ != nullptr) {
      assert(t_->return_address_ == nullptr);
      t_->return_address_ = this;
    }
    return *this;
  }

  T& operator*() { return *get(); }
  T* operator->() { return get(); }
  T* get() { return t_ == nullptr ? nullptr : t_->get(); }

  MovablePointer(const MovablePointer<T>&) = delete;
  MovablePointer<T>& operator=(const MovablePointer<T>&) = delete;

  MovablePointer(MovablePointer<T>&& other) : MovablePointer() {
    swap(*this, other);
  }

  MovablePointer<T>& operator=(MovablePointer<T>&& other) {
    swap(*this, other);
    return *this;
  }

  ~MovablePointer() {
    if (t_ != nullptr) {
      t_->return_address_ = nullptr;
    }
  }

 private:
  MovablePointee<T>* t_;

  friend class MovablePointee<T>;
  friend void swap<>(MovablePointer<T>& a, MovablePointer<T>& b);
  friend void swap<>(MovablePointee<T>& a, MovablePointee<T>& b);
};

#endif /* ifndef MOVABLE_POINTER_H */
