#include <algorithm>
#include <cassert>
#include <type_traits>

template <typename T>
class MovablePointer;

template <typename T>
class MovablePointee {
 public:
  MovablePointee() : return_address_(nullptr) {}

  MovablePointee(const MovablePointee<T>&) = delete;
  MovablePointee<T>& operator=(const MovablePointee<T>&) = delete;

  MovablePointee(MovablePointee<T>&& other) noexcept : MovablePointee() {
    swap(other);
  }

  MovablePointee<T>& operator=(MovablePointee<T>&& other) {
    swap(other);
    return *this;
  }

  void swap(MovablePointee<T>& b) {
    using std::swap;
    swap(return_address_, b.return_address_);
    if (return_address_ != nullptr) {
      return_address_->t_ = static_cast<T*>(this);
    }
    if (b.return_address_ != nullptr) {
      b.return_address_->t_ = static_cast<T*>(&b);
    }
  }

  virtual ~MovablePointee() {
    if (return_address_ != nullptr) {
      return_address_->t_ = nullptr;
    }
  }

 private:
  friend class MovablePointer<T>;

  template <typename U>
  friend void swap(MovablePointer<U>& a, MovablePointer<U>& b);

  MovablePointer<T>* return_address_;
};

template <typename T>
class MovablePointer final {
 public:
  MovablePointer() : t_(nullptr) {
    static_assert(std::is_base_of<MovablePointee<T>, T>::value,
                  "T must be a descendant of MovablePointee<T>");
  }
  explicit MovablePointer(T* t) : MovablePointer() { *this = t; }
  MovablePointer<T>& operator=(T* t) {
    t_ = t;
    if (t_ != nullptr) {
      assert(t_->return_address_ == nullptr);
      t_->return_address_ = this;
    }
    return *this;
  }

  T& operator*() { return *t_; }
  T* operator->() { return t_; }
  T* get() { return t_; }

  MovablePointer(const MovablePointer<T>&) = delete;
  MovablePointer<T>& operator=(const MovablePointer<T>&) = delete;

  MovablePointer(MovablePointer<T>&& other) : MovablePointer() {
    swap(*this, other);
  }

  MovablePointer<T>& operator=(MovablePointer<T>&& other) {
    swap(*this, other);
    return *this;
  }

  template <typename U>
  friend void swap(MovablePointer<U>& a, MovablePointer<U>& b) {
    using std::swap;
    swap(a.t_, b.t_);
    if (a.t_ != nullptr) {
      a.t_->return_address_ = &a;
    }
    if (b.t_ != nullptr) {
      b.t_->return_address_ = &b;
    }
  }

  ~MovablePointer() {
    if (t_ != nullptr) {
      t_->return_address_ = nullptr;
    }
  }

 private:
  T* t_;
  friend class MovablePointee<T>;
};
