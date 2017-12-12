#include <algorithm>
#include <cassert>

template <typename T>
class MovablePointer;

template <typename T>
class MovablePointee : public T {
 public:
  MovablePointee() : T(), return_address_(nullptr) {}
  template <typename... Args>
  MovablePointee(Args&&... args)
      : T(std::forward<Args&&...>(args...)), return_address_(nullptr) {}

  MovablePointee(const MovablePointee<T>&) = delete;
  MovablePointee<T>& operator=(const MovablePointee<T>&) = delete;

  MovablePointee(MovablePointee<T>&& other) noexcept : MovablePointee() {
    swap(*this, other);
  }

  MovablePointee<T>& operator=(MovablePointee<T>&& other) {
    swap(*this, other);
    return *this;
  }

  template <typename U>
  friend void swap(MovablePointee<U>& a, MovablePointee<U>& b) {
    using std::swap;
    swap(static_cast<T&>(a), static_cast<T&>(b));
    swap(a.return_address_, b.return_address_);
    if (a.return_address_ != nullptr) {
      a.return_address_->t_ = &a;
    }
    if (b.return_address_ != nullptr) {
      b.return_address_->t_ = &b;
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
  T* get() { return static_cast<T*>(t_); }

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
  MovablePointee<T>* t_;
  friend class MovablePointee<T>;

  template <typename U>
  friend void swap(MovablePointee<U>& a, MovablePointee<U>& b);
};
