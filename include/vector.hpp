#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include <climits>
#include <cstddef>
#include <cstdlib>

#include "exceptions.hpp"

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T> class vector {
public:
  /**
   * TODO
   * a type for actions of the elements of a vector, and you should write
   *   a class named const_iterator with same interfaces.
   */
  /**
   * you can see RandomAccessIterator at CppReference for help.
   */
  class const_iterator;
  class iterator {
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a
    // type. For instance, for an iterator, iterator::value_type is the type
    // that the iterator points to. STL algorithms and containers may use
    // these type_traits (e.g. the following typedef) to work properly. In
    // particular, without the following code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type:
    // https://blog.csdn.net/u014299153/article/details/72419713 About
    // iterator_category: https://en.cppreference.com/w/cpp/iterator
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    T *ptr;
    T *stat;
    T *end;

  public:
    iterator(T *ptr, T *stat, T *end) : ptr(ptr), stat(stat), end(end) {}
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    iterator operator+(const int &n) const {
      // TODO
      auto result = *this;
      result += n;
      return result;
    }
    iterator operator-(const int &n) const {
      // TODO
      auto result = *this;
      result -= n;
      return result;
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw
    // invaild_iterator.
    int operator-(const iterator &rhs) const {
      // TODO
      auto tmp = this->ptr;
      if (stat != rhs.stat)
        throw index_out_of_bound(); // 不同的vector怎么判断？
      return (tmp - rhs.ptr);
    }
    iterator &operator+=(const int &n) {
      // TODO
      difference_type dif = end - ptr;
      if (dif < n) {
        throw index_out_of_bound();
      }
      ptr += n;
      return *this;
    }
    iterator &operator-=(const int &n) {
      // TODO
      difference_type dif = ptr - stat;
      if (dif < n) {
        throw index_out_of_bound();
      }
      ptr -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      if (end - ptr <= 0) {
        throw index_out_of_bound();
      } else {
        auto result = *this;
        ++result.ptr;
        return result;
      }
    }
    /**
     * TODO ++iter
     */
    iterator &operator++() {
      if (end - ptr <= 0) {
        throw index_out_of_bound();
      } else {
        this->ptr++;
        return *this;
      }
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      if (stat == ptr) {
        throw index_out_of_bound();
      } else {
        auto result = *this;
        --result.ptr;
        return result;
      }
    }
    /**
     * TODO --iter
     */
    iterator &operator--() {
      if (ptr == stat) {
        throw index_out_of_bound();
      } else {
        this->ptr--;
        return *this;
      }
    }
    /**
     * TODO *it
     */
    T &operator*() const { return *ptr; }
    /**
     * a operator to check whether two iterators are same (pointing to the
     * same memory address).
     */
    bool operator==(const iterator &rhs) const { return (ptr == rhs.ptr); }
    bool operator==(const const_iterator &rhs) const {
      return (ptr == rhs.ptr);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return (ptr != rhs.ptr); }
    bool operator!=(const const_iterator &rhs) const {
      return (ptr != rhs.ptr);
    }
  };
  /**
   * TODO
   * has same function as iterator, just for a const object.
   */
  class const_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    const T *ptr;
    const T *stat;
    const T *end;

  public:
    const_iterator(const T *ptr, const T *stat, const T *end)
        : ptr(ptr), stat(stat), end(end) {}
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    const_iterator operator+(const int &n) const {
      // TODO
      auto result = *this;
      result += n;
      return result;
    }
    const_iterator operator-(const int &n) const {
      // TODO
      auto result = *this;
      result -= n;
      return result;
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw
    // invaild_iterator.
    int operator-(const_iterator &rhs) const {
      // TODO
      if (stat != rhs.stat)
        throw index_out_of_bound(); // 不同的vector怎么判断？
      return (ptr - rhs.ptr);
    }
    const_iterator &operator+=(const int &n) {
      // TODO
      difference_type dif = end - ptr;
      if (dif < n) {
        throw index_out_of_bound();
      }
      ptr += n;
      return *this;
    }
    const_iterator &operator-=(const int &n) {
      // TODO
      difference_type dif = ptr - stat;
      if (dif < n) {
        throw index_out_of_bound();
      }
      ptr -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      if (end - ptr <= 0) {
        throw index_out_of_bound();
      } else {
        auto result = *this;
        ++result.ptr;
        return result;
      }
    }
    /**
     * TODO ++iter
     */
    const_iterator &operator++() {
      if (end - ptr <= 0) {
        throw index_out_of_bound();
      } else {
        this->ptr++;
        return *this;
      }
    }
    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      if (stat == ptr) {
        throw index_out_of_bound();
      } else {
        auto result = *this;
        --result.ptr;
        return result;
      }
    }
    /**
     * TODO --iter
     */
    const_iterator &operator--() {
      if (ptr == stat) {
        throw index_out_of_bound();
      } else {
        this->ptr--;
        return *this;
      }
    }
    /**
     * TODO *it
     */
    const T &operator*() const { return *ptr; }
    /**
     * a operator to check whether two iterators are same (pointing to the
     * same memory address).
     */
    bool operator==(const iterator &rhs) const { return (ptr == rhs.ptr); }
    bool operator==(const const_iterator &rhs) const {
      return (ptr == rhs.ptr);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return (ptr != rhs.ptr); }
    bool operator!=(const const_iterator &rhs) const {
      return (ptr != rhs.ptr);
    }
  };
  /**
   * TODO Constructs
   * At least two: default constructor, copy constructor
   */
private:
  T *data;
  size_t maxcapacity;
  size_t curcapacity;
  static const size_t CAP = 256;

  void expansion() {
    const auto tmp = *this;
    maxcapacity *= 2;
    for (int i = 0; i < curcapacity; i++) {
      data[i].~T();
    }
    free(data);
    data = (T *)malloc(sizeof(T) * maxcapacity);
    curcapacity = tmp.curcapacity;
    for (int i = 0; i < curcapacity; i++) {
      new (&data[i]) T(tmp.data[i]);
    }
  }

public:
  vector() {
    maxcapacity = CAP;
    curcapacity = 0;
    data = (T *)malloc(sizeof(T) * maxcapacity);
  }
  vector(const vector &other) {
    maxcapacity = other.maxcapacity;
    curcapacity = other.curcapacity;
    data = (T *)malloc(sizeof(T) * other.maxcapacity);
    for (int i = 0; i < curcapacity; i++) {
      new (&data[i]) T(other.data[i]);
    }
  }
  /**
   * TODO Destructor
   */
  ~vector() {
    for (int i = 0; i < curcapacity; i++) {
      data[i].~T();
    }
    free(data);
    data = nullptr;
  }
  /**
   * TODO Assignment operator
   */
  vector &operator=(const vector &other) {
    if (this == &other) {
      return *this;
    }
    for (int i = 0; i < curcapacity; i++) {
      data[i].~T();
    }
    free(data);
    data = (T *)malloc(sizeof(T) * other.maxcapacity);
    maxcapacity = other.maxcapacity;
    curcapacity = other.curcapacity;
    for (int i = 0; i < curcapacity; i++) {
      new (&data[i]) T(other.data[i]);
    }
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  T &at(const size_t &pos) {
    if (pos < 0 || pos >= curcapacity) {
      throw index_out_of_bound();
    }
    return data[pos];
  }
  const T &at(const size_t &pos) const {
    if (pos < 0 || pos >= curcapacity) {
      throw index_out_of_bound();
    }
    return data[pos];
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
  T &operator[](const size_t &pos) {
    if (pos < 0 || pos >= curcapacity) {
      throw index_out_of_bound();
    }
    return data[pos];
  }
  const T &operator[](const size_t &pos) const {
    if (pos < 0 || pos >= curcapacity) {
      throw index_out_of_bound();
    }
    return data[pos];
  }
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  const T &front() const {
    if (curcapacity == 0) {
      throw container_is_empty();
    }
    return data[0];
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  const T &back() const {
    if (curcapacity == 0) {
      throw container_is_empty();
    }
    return data[curcapacity - 1];
  }
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() { return iterator(data, data, data + curcapacity); }
  const_iterator begin() const {
    return const_iterator(data, data, data + curcapacity);
  }
  const_iterator cbegin() const {
    return const_iterator(data, data, data + curcapacity);
  }
  /**
   * returns an iterator to the end.
   */
  iterator end() {
    return iterator(data + curcapacity, data, data + curcapacity);
  }
  const_iterator end() const {
    return const_iterator(data + curcapacity, data, data + curcapacity);
  }
  const_iterator cend() const {
    return const_iterator(data + curcapacity, data, data + curcapacity);
  }
  /**
   * checks whether the container is empty
   */
  bool empty() const { return curcapacity == 0; }
  /**
   * returns the number of elements
   */
  size_t size() const { return curcapacity; }
  /**
   * clears the contents
   */
  void clear() {
    for (int i = 0; i < curcapacity; i++) {
      data[i].~T();
    }
    curcapacity = 0;
  }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  iterator insert(iterator pos, const T &value) {
    if (pos - this->begin() < 0 || this->end() - pos < 0) {
      throw index_out_of_bound();
    }
    size_t index = pos - this->begin();
    if (curcapacity > index) {
      new (&data[curcapacity]) T(data[curcapacity - 1]);
      for (int i = curcapacity - 1; i >= (int)(index + 1); i--) {
        data[i] = data[i - 1];
      }
      data[index] = value;
    } else {
      // insert at end
      new (&data[index]) T(value);
    }
    curcapacity++;
    if (curcapacity + 1 == maxcapacity) {
      expansion();
    }
    return pos;
  }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size
   * because after inserting the size will increase 1.)
   */
  iterator insert(const size_t &ind, const T &value) {
    iterator tmpptr(data + ind, data, data + ind);
    return insert(tmpptr, value);
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is
   * returned.
   */
  iterator erase(iterator pos) {
    if (pos - this->begin() < 0 || this->end() - pos < 0) {
      throw index_out_of_bound();
    }
    size_t index = pos - this->begin();
    for (size_t i = index; i < curcapacity - 1; i++) {
      data[i] = data[i + 1];
    }
    curcapacity--;
    return pos;
  }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  iterator erase(const size_t &ind) {
    iterator tmpptr(data + ind, data, data + ind);
    return erase(tmpptr);
  }
  /**
   * adds an element to the end.
   */
  void push_back(const T &value) {
    new (&data[curcapacity++]) T(value);
    if (curcapacity + 1 == maxcapacity) {
      expansion();
    }
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  void pop_back() {
    if (curcapacity == 0) {
      throw container_is_empty();
    }
    data[curcapacity - 1].~T();
    curcapacity--;
  }
};

} // namespace sjtu

#endif