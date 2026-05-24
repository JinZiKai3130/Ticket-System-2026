#pragma once
#include <cstddef>
#include <utility>

template <typename T> class shared_ptr {
public:
  shared_ptr(T *ptr = nullptr)
      : ptr_(ptr), count(ptr ? new std::size_t(1) : nullptr) {}

  shared_ptr(const shared_ptr &other) : ptr_(other.ptr_), count(other.count) {
    if (count) {
      ++(*count);
    }
  }

  shared_ptr(shared_ptr &&other) : ptr_(other.ptr_), count(other.count) {
    other.ptr_ = nullptr;
    other.count = nullptr;
  }

  shared_ptr &operator=(const shared_ptr &other) {
    if (this != &other) {
      release();
      ptr_ = other.ptr_;
      count = other.count;
      if (count) {
        ++(*count);
      }
    }
    return *this;
  }

  shared_ptr &operator=(shared_ptr &&other) {
    if (this != &other) {
      release();
      ptr_ = other.ptr_;
      count = other.count;
      other.ptr_ = nullptr;
      other.count = nullptr;
    }
    return *this;
  }

  ~shared_ptr() { release(); }

  T &operator*() const { return *ptr_; }

  T *operator->() const { return ptr_; }

  std::size_t use_count() const { return count ? *count : 0; }

private:
  T *ptr_;
  std::size_t *count;

  void release() {
    if (count) {
      --(*count);
      if (*count == 0) {
        delete ptr_;
        delete count;
      }
    }
    ptr_ = nullptr;
    count = nullptr;
  }
};

template <typename T> shared_ptr<T> make_shared(const T &arg) {
  return shared_ptr<T>(new T(arg));
}