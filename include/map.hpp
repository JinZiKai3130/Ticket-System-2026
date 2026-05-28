/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <cstddef>
#include <functional>

#include "exceptions.hpp"
#include "utility.hpp"

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key>> class map {
public:
  typedef pair<const Key, T> value_type;

private:
  struct Node {
    value_type data;
    Node *lc;
    Node *rc;
    Node *fa;
    int height;
    Node(const value_type &element, Node *lc, Node *rc, Node *fa, int h = 0)
        : data(element), lc(lc), rc(rc), fa(fa), height(h) {}
  };
  Node *root;
  Compare comp;
  size_t capacity;

  void destroy(Node *cur) {
    if (cur == nullptr)
      return;
    destroy(cur->lc);
    destroy(cur->rc);
    delete cur;
  }

  Node *clone(Node *other, Node *parent) {
    if (other == nullptr)
      return nullptr;
    Node *p = new Node(other->data, nullptr, nullptr, parent, other->height);
    try {
      p->lc = clone(other->lc, p);
      p->rc = clone(other->rc, p);
    } catch (...) {
      destroy(p);
      throw;
    }
    return p;
  }

  int height(Node *cur) { return (cur == nullptr) ? 0 : cur->height; }

  Node *insert_node(Node *node, Node *parent, const value_type &value,
                    bool &inserted, Node *&out_node) {
    if (!node) { // 当前树为空
      inserted = true;
      out_node = new Node(value, nullptr, nullptr, parent, 0);
      return out_node;
    }
    node->fa = parent;
    if (comp(value.first, node->data.first)) {
      node->lc = insert_node(node->lc, node, value, inserted, out_node);
    } else if (comp(node->data.first, value.first)) {
      node->rc = insert_node(node->rc, node, value, inserted, out_node);
    } else {
      inserted = false;
      out_node = node;
      return node;
    }
    node = rebalance(node);
    node->fa = parent;
    return node;
  }

  Node *rebalance(Node *node) {
    if (!node)
      return node;
    node->height = std::max(height(node->lc), height(node->rc)) + 1;
    int balance = height(node->lc) - height(node->rc);
    if (balance > 1) {
      if (height(node->lc->lc) - height(node->lc->rc) >= 0) {
        node = LL(node); // 树右旋
      } else {
        node->lc = RR(node->lc); // 左子树先左旋
        if (node->lc)
          node->lc->fa = node;
        node = LL(node); // 树右旋
      }
    } else if (balance < -1) {
      if (height(node->rc->rc) - height(node->rc->lc) >= 0) {
        node = RR(node);
      } else {
        node->rc = LL(node->rc);
        if (node->rc)
          node->rc->fa = node;
        node = RR(node);
      }
    }
    return node;
  }

  Node *find_min(Node *node, Node *parent, Node *&min_node) {
    if (!node)
      return nullptr;
    node->fa = parent;
    if (!node->lc) { // lc是空，i.e.已经找到
      min_node = node;
      Node *right_child = node->rc;
      if (right_child)
        right_child->fa = node->fa;
      min_node->lc = nullptr;
      min_node->rc = nullptr;
      return right_child;
    }
    node->lc = find_min(node->lc, node, min_node);
    node = rebalance(node);
    node->fa = parent;
    return node;
  }

  Node *erase_node(Node *node, Node *parent, const Key &key, bool &erased) {
    // std::cerr << "erase_node now\n";
    if (!node)
      return nullptr;
    node->fa = parent;
    if (comp(key, node->data.first)) {
      node->lc = erase_node(node->lc, node, key, erased);
    } else if (comp(node->data.first, key)) {
      node->rc = erase_node(node->rc, node, key, erased);
    } else {
      erased = true;
      if (!node->lc && !node->rc) { // 没有子节点
        // std::cerr << "no subtree\n";
        delete node;
        return nullptr;
      } else if (!node->lc || !node->rc) { // 1个子节点
        Node *only_child = node->lc ? node->lc : node->rc;
        only_child->fa = node->fa; // 也可以是parent
        delete node;
        return only_child;
      } else { // 2个子节点
        Node *min_node;
        node->rc = find_min(node->rc, node, min_node);
        // 找右子树上的最小节点作为min_node，用于替代原根节点，返回出右子树的新的右子树根节点
        min_node->lc = node->lc;
        if (min_node->lc)
          min_node->lc->fa = min_node;
        min_node->rc = node->rc;
        if (min_node->rc)
          min_node->rc->fa = min_node;
        min_node->fa = parent;
        delete node;
        node = min_node;
      }
    }
    node = rebalance(node);
    node->fa = parent;
    return node;
  }

  Node *LL(Node *&t) {
    Node *tl = t->lc;
    t->lc = tl->rc;
    if (t->lc)
      t->lc->fa = t;
    tl->rc = t;
    tl->fa = t->fa;
    t->fa = tl;
    t->height = std::max(height(t->lc), height(t->rc)) + 1;
    tl->height = std::max(height(tl->lc), height(tl->rc)) + 1;
    t = tl;
    return t;
  }

  Node *RR(Node *&t) {
    Node *tr = t->rc;
    t->rc = tr->lc;
    if (t->rc)
      t->rc->fa = t;
    tr->lc = t;
    tr->fa = t->fa;
    t->fa = tr;
    t->height = std::max(height(t->lc), height(t->rc)) + 1;
    tr->height = std::max(height(tr->lc), height(tr->rc)) + 1;
    t = tr;
    return t;
  }

  // void LR(Node*& t) {
  //     RR(t->lc);
  //     LL(t);
  // }

  // void RL(Node*& t) {
  //     LL(t->rc);
  //     RR(t);
  // }

public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  // typedef pair<const Key, T> value_type;

  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
  public:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    Node *cur;
    const map *id;
    iterator() : cur(nullptr), id(nullptr) {}
    iterator(Node *cur, const map *id) : cur(cur), id(id) {}
    iterator(const iterator &other) : cur(other.cur), id(other.id) {}

    /**
     * TODO iter++
     */
    iterator &operator++() {
      if (!cur)
        throw invalid_iterator();

      if (cur->rc) {
        cur = cur->rc;
        while (cur->lc)
          cur = cur->lc;
      } else {
        Node *p = cur;
        Node *parent = p->fa;
        while (parent && p == parent->rc) {
          p = parent;
          parent = parent->fa;
        }
        cur = parent;
      }
      return *this;
    }

    /**
     * TODO ++iter
     */
    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    /**
     * TODO iter--
     */
    iterator &operator--() {
      if (!cur) { // end的情况
        if (!id || !id->root)
          throw invalid_iterator();
        cur = id->root;
        while (cur->rc)
          cur = cur->rc; // find the last element
        return *this;
      }
      if (cur->lc) {
        cur = cur->lc;
        while (cur->rc)
          cur = cur->rc;
      } else {
        Node *p = cur;
        Node *parent = p->fa;
        while (parent && p == parent->lc) {
          p = parent;
          parent = parent->fa;
        }
        if (!parent)
          throw invalid_iterator(); // cur = begin
        cur = parent;
      }
      return *this;
    }

    /**
     * TODO --iter
     */
    iterator operator--(int) {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the
     * same memory).
     */
    value_type &operator*() const {
      if (!cur)
        throw invalid_iterator();
      return cur->data;
    }

    bool operator==(const iterator &rhs) const {
      return (cur == rhs.cur) && (id == rhs.id);
    }

    bool operator==(const const_iterator &rhs) const {
      return (cur == rhs.cur) && (id == rhs.id);
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

    bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

    /**
     * for the support of it->first.
     * See
     * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
     * for help.
     */
    value_type *operator->() const noexcept { return &(cur->data); }

    const map *get_id() { return id; }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
  public:
    // data members.
    Node *cur;
    const map *id;

    const_iterator() : cur(nullptr), id(nullptr) {}
    const_iterator(Node *cur, const map *id) : cur(cur), id(id) {}
    const_iterator(const const_iterator &other)
        : cur(other.cur), id(other.id) {}
    const_iterator(const iterator &other) : cur(other.cur), id(other.id) {}

    /**
     * TODO iter++
     */
    const_iterator &operator++() {
      if (!cur)
        throw invalid_iterator();

      if (cur->rc) {
        cur = cur->rc;
        while (cur->lc)
          cur = cur->lc;
      } else {
        Node *p = cur;
        Node *parent = p->fa;
        while (parent && p == parent->rc) {
          p = parent;
          parent = parent->fa;
        }
        cur = parent;
      }
      return *this;
    }

    /**
     * TODO ++iter
     */
    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    /**
     * TODO iter--
     */
    const_iterator &operator--() {
      if (!cur) { // end的情况
        if (!id || !id->root)
          throw invalid_iterator();
        cur = id->root;
        while (cur->rc)
          cur = cur->rc; // find the last element
        return *this;
      }
      if (cur->lc) {
        cur = cur->lc;
        while (cur->rc)
          cur = cur->rc;
      } else {
        Node *p = cur;
        Node *parent = p->fa;
        while (parent && p == parent->lc) {
          p = parent;
          parent = parent->fa;
        }
        if (!parent)
          throw invalid_iterator(); // cur = begin
        cur = parent;
      }
      return *this;
    }

    /**
     * TODO --iter
     */
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the
     * same memory).
     */
    const value_type &operator*() const {
      if (!cur)
        throw invalid_iterator();
      return cur->data;
    }

    bool operator==(const iterator &rhs) const {
      return (cur == rhs.cur) && (id == rhs.id);
    }

    bool operator==(const const_iterator &rhs) const {
      return (cur == rhs.cur) && (id == rhs.id);
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

    bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

    /**
     * for the support of it->first.
     * See
     * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
     * for help.
     */
    const value_type *operator->() const noexcept { return &(cur->data); }

    const map *get_id() { return id; }
  };

public:
  /**
   * TODO two constructors
   */
  map() : root(nullptr), capacity(0), comp(Compare()) {}

  map(const map &other) {
    root = clone(other.root, nullptr);
    comp = other.comp;
    capacity = other.capacity;
  }

  /**
   * TODO assignment operator
   */
  map &operator=(const map &other) {
    if (this == &other)
      return *this;
    Node *tmp = clone(other.root, nullptr);
    destroy(root);
    root = tmp;
    comp = other.comp;
    capacity = other.capacity;
    return *this;
  }

  /**
   * TODO Destructors
   */
  ~map() {
    destroy(root);
    root = nullptr;
    capacity = 0;
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key
   * equivalent to key. If no such element exists, an exception of type
   * `index_out_of_bound'
   */
  T &at(const Key &key) {
    iterator it = find(key);
    if (it == end())
      throw index_out_of_bound();
    return it->second;
  }

  const T &at(const Key &key) const {
    const_iterator it = find(key);
    if (it == cend())
      throw index_out_of_bound();
    return it->second;
  }

  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to
   * key, performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    iterator it = find(key);
    if (it != end())
      return it->second;
    value_type val(key, T());
    pair<iterator, bool> res = insert(val);
    return res.first->second;
  }

  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    const_iterator it = find(key);
    if (it == cend())
      throw index_out_of_bound();
    return it->second;
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    Node *p = root;
    if (!p)
      return iterator(nullptr, this);
    while (p->lc != nullptr) {
      p = p->lc;
    }
    return iterator(p, this);
  }

  const_iterator cbegin() const {
    Node *p = root;
    if (!p)
      return const_iterator(nullptr, this);
    while (p->lc != nullptr) {
      p = p->lc;
    }
    return const_iterator(p, this);
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(nullptr, this); }

  const_iterator cend() const { return const_iterator(nullptr, this); }

  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const { return (capacity == 0); }

  /**
   * returns the number of elements.
   */
  size_t size() const { return capacity; }

  /**
   * clears the contents
   */
  void clear() {
    destroy(root);
    root = nullptr;
    capacity = 0;
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    bool inserted = false;
    Node *out_node = nullptr;
    root = insert_node(root, nullptr, value, inserted, out_node);
    if (root)
      root->fa = nullptr;
    if (inserted)
      capacity++;
    return pair<iterator, bool>(iterator(out_node, this), inserted);
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points
   * an element out of this)
   */
  void erase(iterator pos) {
    if (pos == end() || pos.get_id() != this)
      throw invalid_iterator();
    bool erased = false;
    Key key = (*pos).first;
    root = erase_node(root, nullptr, key, erased);

    if (root)
      root->fa = nullptr;
    if (!erased)
      throw invalid_iterator();
    capacity--;
    // std::cerr << "capacity " << capacity << "\n";
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    const_iterator it = find(key);
    // std::cerr << "find ok\n";
    if (it == cend()) {
      // std::cerr << "here0\n";
      return 0;
    } else
      return 1;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    Node *cur = root;
    while (cur != nullptr) {
      if (comp(key, cur->data.first)) {
        cur = cur->lc;
      } else if (comp(cur->data.first, key)) {
        cur = cur->rc;
      } else {
        break;
      }
    }
    return iterator(cur, this);
  }

  const_iterator find(const Key &key) const {
    Node *cur = root;
    while (cur != nullptr) {
      if (comp(key, cur->data.first)) {
        cur = cur->lc;
      } else if (comp(cur->data.first, key)) {
        cur = cur->rc;
      } else {
        break;
      }
    }
    return const_iterator(cur, this);
  }
};

} // namespace sjtu

#endif