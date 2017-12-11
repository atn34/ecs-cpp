#include <algorithm>
#include <cassert>
#include <type_traits>

template <typename T>
class MovablePointer;

template <typename T>
class MovablePointee {
 public:
  virtual ~MovablePointee() {
    if (return_address_ != nullptr) {
      return_address_->t_ = nullptr;
    }
  }

  void swap(MovablePointee<T>& b) {
    if (return_address_ != nullptr) {
      return_address_->t_ = static_cast<T*>(&b);
    }
    if (b.return_address_ != nullptr) {
      b.return_address_->t_ = static_cast<T*>(this);
    }
  }

 private:
  MovablePointer<T>* return_address_ = nullptr;
  friend class MovablePointer<T>;
};

template <typename T>
class MovablePointer final {
 public:
  explicit MovablePointer(T* t) : t_(t) {
    static_assert(std::is_base_of<MovablePointee<T>, T>::value,
                  "T must be a descendant of MovablePointee<T>");
    assert(t_->return_address_ == nullptr);
    t_->return_address_ = this;
  }

  T& operator*() { return *t_; }
  T* operator->() { return t_; }
  T* get() { return t_; }

  MovablePointer(const MovablePointer<T>&) = delete;

  ~MovablePointer() {
    if (t_ != nullptr) {
      t_->return_address_ = nullptr;
    }
  }

 private:
  T* t_;
  friend class MovablePointee<T>;
};
