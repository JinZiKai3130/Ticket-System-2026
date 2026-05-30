#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cmath>      // in case you need it
#include <cstddef>    // for size_t
#include <functional> // for std::less

#include "exceptions.hpp"

namespace sjtu {

/**
 * @brief A container automatically sorting its contents, similar to
 * std::priority_queue but with extra functionalities.
 *
 * The extra functionalities are:
 * - Merge two priority queues into one (with good time complexity).
 * - Clear all elements in the queue.
 * - Limited exception safety for some operations (e.g. push, pop, top, merge)
 * when the comparator throws exceptions from `Compare` only.
 *
 * This @priority_queue does not support passing an underlying container as a
 * template parameter. Also, it does not support passing a comparator object as
 * a constructor argument.
 *
 */
template <class T, class Compare = std::less<T>> class priority_queue {
private:
  struct Node {
    T val;
    Node *l;
    Node *r;
    int dis;
    Node(const T &v) : val(v), l(nullptr), r(nullptr), dis(0) {}
  };
  Node *root;
  size_t len;
  Compare comp;
  Node *merge_node(Node *a, Node *b) {
    if (!a)
      return b;
    if (!b)
      return a;
    if (comp(a->val, b->val))
      std::swap(a, b);
    a->r = merge_node(a->r, b);
    int dis_l = (a->l ? a->l->dis : -1);
    int dis_r = (a->r ? a->r->dis : -1);
    if (dis_l < dis_r)
      std::swap(a->l, a->r);
    a->dis = (a->r ? a->r->dis + 1 : 0);
    return a;
  }

  Node *clone(Node *other) {
    if (!other)
      return nullptr;
    Node *p = new Node(other->val);
    p->dis = other->dis;
    try {
      p->l = clone(other->l);
      p->r = clone(other->r);
    } catch (...) {
      destroy(p);
      throw;
    }
    return p;
  }

  void destroy(Node *cur) {
    if (!cur)
      return;
    destroy(cur->l);
    destroy(cur->r);
    delete cur;
  }

public:
  priority_queue() : root(nullptr), len(0), comp(Compare()) {}
  priority_queue(const priority_queue &other)
      : root(clone(other.root)), len(other.len), comp(other.comp) {}
  ~priority_queue() { destroy(root); }

  priority_queue &operator=(const priority_queue &other) {
    if (this == &other)
      return *this;
    Node *tmp = clone(other.root);
    destroy(root);
    root = tmp;
    len = other.len;
    comp = other.comp;
    return *this;
  }

  /** Adds one element to the queue. */
  void push(const T &cur) {
    Node *p = new Node(cur);
    try {
      root = merge_node(root, p);
      ++len;
    } catch (...) {
      delete p;
      throw;
    }
  }

  /**
   * Returns a read-only reference of the first element in the queue.
   *
   * @throws container_is_empty when the first element does not exist.
   */
  const T &top() const {
    if (empty())
      throw container_is_empty();
    return root->val;
  }

  /**
   * Removes the first element in the queue.
   * @throws container_is_empty when the first element does not exist.
   */
  void pop() {
    if (empty())
      throw container_is_empty();
    Node *old = root;
    root = merge_node(root->l, root->r);
    delete old;
    --len;
  }

  /** Returns the number of elements in the queue. */
  size_t size() const { return len; }

  /** Returns whether there is any element in the queue. */
  bool empty() const { return (len == 0); }

  /** Clears all elements in the queue. */
  void clear() {
    destroy(root);
    len = 0;
    root = nullptr;
  }

  /**
   * @brief Merges two priority queues into one.
   *
   * The merged data shall be stored in the current priority queue and the
   * other priority queue shall be cleared after merging.
   *
   * The time complexity shall be O(log n) or better.
   */
  void merge(priority_queue &other) {
    if (this == &other || other.empty())
      return;
    root = merge_node(root, other.root);
    len += other.len;
    other.root = nullptr;
    other.len = 0;
  }
};

} // namespace sjtu

#endif