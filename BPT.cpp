#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;

template <typename T, int info_len = 2> class MemoryRiver {
private:
  fstream file;
  string file_name;
  int sizeofT = sizeof(T);

public:
  MemoryRiver() = default;

  MemoryRiver(const string &file_name) : file_name(file_name) {}

  void initialise(string FN = "") {
    if (FN != "")
      file_name = FN;
    file.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
    int tmp = 0;
    for (int i = 0; i < info_len; ++i)
      file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
  }

  // 读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len)
      return;
    file.open(file_name, std::ios::in | std::ios::binary);
    file.seekg((n - 1) * sizeof(int), std::ios::beg);
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
  }

  // 将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len)
      return;
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp((n - 1) * sizeof(int), std::ios::beg);
    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
  }

  // 在文件合适位置写入类对象t，并返回写入的位置索引index
  // 位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
  // 位置索引index可以取为对象写入的起始位置
  int write(T &t) {
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(0, std::ios::end);
    int index = file.tellp();
    file.write(reinterpret_cast<const char *>(&t), sizeof(T));
    file.close();
    return index;
  }

  // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(T &t, const int index) {
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(index, std::ios::beg);
    file.write(reinterpret_cast<const char *>(&t), sizeof(T));
    file.close();
  }

  // 读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, const int index) {
    file.open(file_name, std::ios::in | std::ios::binary);
    file.seekg(index, std::ios::beg);
    file.read(reinterpret_cast<char *>(&t), sizeof(T));
    file.close();
  }

  // 删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
  void Delete(int index) {}
};

template <typename T> class BPT {
  static const int ORDER = 128; // 128

public:
  struct index_value {
    char index[65] = {};
    T value = T();

    bool operator<(const index_value &other) const {
      int cmp = strcmp(this->index, other.index);
      if (cmp < 0)
        return true;
      else if (cmp > 0)
        return false;
      else {
        return value < other.value;
      }
    }

    bool operator==(const index_value &other) const {
      return (strcmp(this->index, other.index) == 0 &&
              this->value == other.value);
    }

    bool operator<=(const index_value &other) const {
      return (*this < other) || (*this == other);
    }

    bool operator!=(const index_value &other) const {
      return !(*this == other);
    }

    index_value() {}
  };

private:
  struct Node {
    int fa = 0;
    int child[ORDER + 5]{}; // 0-127
    size_t count = 0;
    index_value element[ORDER + 5]{};
    // 1-127 这里预留空间+5保证不会split之前超出限制
    bool is_leaf = 1;
    int next = 0;

    Node() {}
  };

  MemoryRiver<Node, 5> tree_node;

public:
  int head;
  int root;

  void init() {
    ifstream test("node_data", std::ios::binary);
    bool exists = test.good();
    test.close();
    if (!exists) {
      tree_node.initialise("node_data");
      Node initial;
      int address = tree_node.write(initial);
      root = address;
      head = address;
      tree_node.write_info(root, 1);
      tree_node.write_info(head, 2);
      return;
    }
    tree_node = MemoryRiver<Node, 5>("node_data");
    tree_node.get_info(root, 1);
    tree_node.get_info(head, 2);
    if (root == 0) {
      tree_node.initialise("node_data");
      Node initial;
      int address = tree_node.write(initial);
      root = address;
      head = address;
      tree_node.write_info(root, 1);
      tree_node.write_info(head, 2);
    }
  }

  bool findinterval(const int &pos, const index_value &target, int *vec,
                    int &num) {
    Node cur_node;
    tree_node.read(cur_node, pos);
    int offset = 0;
    num = 0;
    if (cur_node.is_leaf) {
      // std::cout << "leaf_now\n";
      for (int i = 1; i <= cur_node.count; i++) {
        if (strcmp(target.index, cur_node.element[i].index) == 0) {
          offset = i;
          break;
        }
      }
      if (offset == 0) { // 有可能是下一组的第一个
        int next_index = cur_node.next;
        if (next_index == 0) {
          return false;
        }
        tree_node.read(cur_node, next_index);
        if (strcmp(target.index, cur_node.element[1].index) == 0) {
          offset = 1;
        }
      }
      if (offset != 0) { // 确实存在这个index

        // std::cout << "yes we find the target index\n";
        int i = offset;
        while (i <= cur_node.count) {
          // 当前element index仍然和搜索的要求一致
          // std::cout << "index = " << cur_node.element[i].index << std::endl;
          if (strcmp(cur_node.element[i].index, target.index) == 0) {
            vec[++num] = cur_node.element[i].value;
            // 需要继续进入下一个块读入，从当前块开始读
            if (i == cur_node.count) {
              tree_node.read(cur_node, cur_node.next);
              if (cur_node.next == 0) {
                break;
              }
              // std::cout << "get_next\n";
              // std::cout << "next_first_index = " << cur_node.element[1].index
              //           << std::endl;
              // std::cout << "next_count = " << cur_node.count << std::endl;
              // std::cout << "next_first_value = " << cur_node.element[1].value
              //           << std::endl;
              i = 1;
            } else {
              i++;
            }
            // std::cout << "num = " << num << std::endl;
            // std::cout << "num++";
          } else {
            break;
          }
        }
        return true;
      } else {
        return false;
      }
    }

    // 现在还不是叶子节点
    // std::cout << "not leaf now\n";
    if (target < cur_node.element[1]) {
      return findinterval(cur_node.child[0], target, vec, num);
    }
    for (int i = 1; i < cur_node.count; i++) {
      if (cur_node.element[i] <= target && target < cur_node.element[i + 1]) {
        return findinterval(cur_node.child[i], target, vec, num);
      }
    }
    return findinterval(cur_node.child[cur_node.count], target, vec, num);
  }

  void findpoint(const int &pos, const index_value &target,
                 std::shared_ptr<Node> &out_node, int &node_num, int &offset) {
    Node *cur_node = new Node;
    tree_node.read(*cur_node, pos);
    // 交给tmp_out_node实现自动管理
    std::shared_ptr<Node> tmp_out_node(cur_node);

    // 这里的offset表示当前插入的element数组中的index，或者删除的element的index
    if (cur_node->is_leaf) {
      // std::cout << "cur_node.count = " << cur_node->count << "\n";
      out_node = tmp_out_node;
      node_num = pos;
      if (cur_node->count == 0) {
        offset = 1;
        return;
      }
      if (target <= cur_node->element[1]) {
        offset = 1;
      } else if (cur_node->element[cur_node->count] < target) {
        offset = cur_node->count + 1;
        // std::cout << "2 offset = " << offset << std::endl;
      } else {
        for (int i = 1; i < cur_node->count; i++) {
          if (cur_node->element[i] < target &&
              target <= cur_node->element[i + 1]) {
            offset = i + 1;
            // std::cout << "1 offset = " << offset << std::endl;
            return;
          }
        }
      }
      return;
    }

    // 如果不是叶节点，则根据大小比较，继续向下寻找
    if (target < cur_node->element[1]) {
      findpoint(cur_node->child[0], target, out_node, node_num, offset);
      return;
    } else if (cur_node->element[cur_node->count] <= target) {
      findpoint(cur_node->child[cur_node->count], target, out_node, node_num,
                offset);
    } else {
      for (int i = 1; i < cur_node->count; i++) {
        if (cur_node->element[i] <= target &&
            target < cur_node->element[i + 1]) {
          findpoint(cur_node->child[i], target, out_node, node_num, offset);
          return;
        }
      }
    }
  }

  void split(std::shared_ptr<Node> cur_node, const int &node_num) {
    // 如果是根节点，则申请空的节点，然后其他依旧保持一致
    if (node_num == root) {
      // std::cout << "there is the new root\n";
      Node tmp_fa;
      tmp_fa.child[0] = node_num;
      tmp_fa.is_leaf = false;
      cur_node->fa = tree_node.write(tmp_fa);
      root = cur_node->fa;
      tree_node.write_info(root, 1);
    }

    // 读出父节点
    index_value min_of_child = cur_node->element[1];
    Node *father_node = new Node;
    tree_node.read(*father_node, cur_node->fa);

    // 找到父节点需要插入元素的位置
    int father_offset;
    if (father_node->count == 0) {
      father_offset = 1;
    } else if (min_of_child < father_node->element[1]) {
      father_offset = 1;
    } else if (father_node->element[father_node->count] <= min_of_child) {
      father_offset = father_node->count + 1;
    } else {
      for (int i = 1; i < father_node->count; i++) {
        if (father_node->element[i] <= min_of_child &&
            min_of_child < father_node->element[i + 1]) {
          father_offset = i + 1;
          break;
        }
      }
    }
    // std::cout << "father_offset = " << father_offset << '\n';
    // 找到当前节点分离的位置
    int selected_offset = ceil((double)(ORDER + 1) / 2.0);

    // 调整父节点的元素，并加入新的数值
    for (int i = father_node->count; i >= father_offset; i--) {
      father_node->element[i + 1] = father_node->element[i];
      father_node->child[i + 1] = father_node->child[i];
    }
    father_node->element[father_offset] = cur_node->element[selected_offset];
    father_node->count++;

    // 新得到节点tmp，然后写入根据是否是叶节点，决定是否需要调整child，以及如何调整element
    Node tmp;
    tmp.fa = cur_node->fa;
    tmp.is_leaf = cur_node->is_leaf;
    if (tmp.is_leaf) {
      tmp.next = cur_node->next;
      tmp.count = cur_node->count - selected_offset + 1;
      for (int i = selected_offset; i <= cur_node->count; i++) {
        tmp.element[i - selected_offset + 1] = cur_node->element[i];
      }
    } else {
      tmp.next = 0;
      for (int i = selected_offset; i <= cur_node->count; i++) {
        tmp.child[i - selected_offset] = cur_node->child[i];
      }
      tmp.count = cur_node->count - selected_offset;
      for (int i = selected_offset + 1; i <= cur_node->count; i++) {
        tmp.element[i - selected_offset] = cur_node->element[i];
      }
    }
    // 将分裂出来的两个块写入文件
    int new_address = tree_node.write(tmp);
    if (tmp.is_leaf) {
      cur_node->next = new_address;
    }
    cur_node->count = selected_offset - 1;
    tree_node.update(*cur_node, node_num);

    // father_node更新，注意，这里先不写入文件，因为有可能继续上溢出
    father_node->child[father_offset] = new_address;
    // child更新
    if (!tmp.is_leaf) {
      for (int i = 0; i <= tmp.count; i++) {
        Node tmp_child;
        tree_node.read(tmp_child, tmp.child[i]);
        tmp_child.fa = new_address;
        tree_node.update(tmp_child, tmp.child[i]);
      }
    }

    std::shared_ptr<Node> tmp_fa(father_node);
    checkinsert(tmp_fa, cur_node->fa);
  }

  void checkinsert(std::shared_ptr<Node> cur_node, const int &node_num) {
    if (cur_node->count <= ORDER) { // 没有问题
      tree_node.update(*cur_node, node_num);
      return;
    }
    split(cur_node, node_num);
  }

  void insert(const index_value &target) {
    // step1 找位置
    std::shared_ptr<Node> cur_node; // 先找到叶节点
    int offset = 1;
    int node_num = 0;
    // std::cout << "finding point\n";
    findpoint(root, target, cur_node, node_num, offset);

    // step2 内存中更新
    // std::cout << "update in memory\n";
    for (int i = cur_node->count; i >= offset; i--) {
      cur_node->element[i + 1] = cur_node->element[i];
    }
    cur_node->element[offset] = target;
    cur_node->count++;

    // std::cout << "checkinsert\n";
    // step3 检查是否上溢，如果没有则写入，如果有，进行相应调整
    checkinsert(cur_node, node_num);

    // for (int i = 1; i <= cur_node->count; i++) {
    //   std::cout << cur_node->element[i].index << " "
    //             << cur_node->element[i].value << std::endl;
    // }
  }

  void update_non_leaf_node(std::shared_ptr<Node> cur_node, const int &node_num,
                            const index_value &to_be_removed,
                            const index_value &to_replace) {
    int cur_node_num = node_num;
    Node buffer;
    while (true) {
      if (cur_node_num == root && !cur_node->is_leaf) { // 仅有可能是全局最小值
        tree_node.read(buffer, root);
        // 看看是否在当前的node中
        for (int i = 1; i <= buffer.count; ++i) {
          if (buffer.element[i] == to_be_removed) {
            buffer.element[i] = to_replace;
            tree_node.update(buffer, root);
            // std::cout << "found on root   offset = " << i << "\n";
            break;
          }
        }
        break;
      }

      // 找到father，并读出来
      tree_node.read(buffer, cur_node->fa);

      // 判断是否是当前层交换的位置
      if (to_be_removed < buffer.element[1]) { // 不是当前层继续向上
        cur_node_num = cur_node->fa;
        cur_node = std::make_shared<Node>(buffer);
        continue;
      } else {
        for (int i = 1; i <= buffer.count; i++) { // 是当前层，直接交换
          if (buffer.element[i] == to_be_removed) {
            buffer.element[i] = to_replace;
            tree_node.update(buffer, cur_node->fa);
            // std::cout << "found on other   offset = " << i << "\n";
            break;
          }
        }
        break;
      }
    }
  }

  void borrow(std::shared_ptr<Node> require_node, const int &require_node_num,
              std::shared_ptr<Node> offer_node, const int &offer_node_num,
              std::shared_ptr<Node> father_node, const int &father_node_num,
              const int &father_node_offset, const bool &borrow_from_left) {
    if (require_node->is_leaf) {
      if (borrow_from_left) { // 从左侧借
        for (int i = require_node->count; i >= 1; i++) {
          require_node->element[i + 1] = require_node->element[i];
        }
        require_node->element[1] = offer_node->element[offer_node->count];
        require_node->count++;
        offer_node->count--;

        father_node->element[father_node_offset] = require_node->element[1];
      } else { // 从右侧借
        require_node->element[++require_node->count] = offer_node->element[1];
        for (int i = 1; i < offer_node->count; i++) {
          offer_node->element[i] = offer_node->element[i + 1];
        }
        offer_node->count--;

        father_node->element[father_node_offset + 1] = require_node->element[1];
      }
    } else {
      Node tmp_child;
      int tmp_child_num;
      if (borrow_from_left) {
        for (int i = require_node->count; i >= 1; i++) {
          require_node->element[i + 1] = require_node->element[i];
          require_node->child[i + 1] = require_node->child[i];
        }
        require_node->child[1] = require_node->child[0];
        require_node->element[1] = father_node->element[father_node_offset];
        require_node->child[0] = offer_node->child[offer_node->count];
        father_node->element[father_node_offset] =
            offer_node->element[offer_node->count];
        require_node->count++;
        offer_node->count--;

        // 还要修改require_node->child[0]
        tree_node.read(tmp_child, require_node->child[0]);
        tmp_child_num = require_node->child[0];
        tmp_child.fa = require_node_num;
      } else {
        require_node->element[++require_node->count] =
            father_node->element[father_node_offset + 1];
        father_node->element[father_node_offset + 1] = offer_node->element[1];
        require_node->child[require_node->count] = offer_node->child[0];
        offer_node->child[0] = offer_node->child[1];
        for (int i = 1; i < offer_node->count; i++) {
          offer_node->element[i] = offer_node->element[i + 1];
          offer_node->child[i] = offer_node->child[i + 1];
        }
        offer_node->count--;

        tree_node.read(tmp_child, require_node->child[require_node->count]);
        tmp_child_num = require_node->child[require_node->count];
        tmp_child.fa = require_node_num;
      }
      tree_node.update(tmp_child, tmp_child_num);
    }
    tree_node.update(*require_node, require_node_num);
    tree_node.update(*offer_node, offer_node_num);
    // tree_node.update(*father_node, father_node_num);
    checkremove(father_node, father_node_num);
  }

  void merge(std::shared_ptr<Node> require_node, int require_node_num,
             std::shared_ptr<Node> offer_node, int offer_node_num,
             std::shared_ptr<Node> father_node, const int &father_node_num,
             int father_node_offset, const bool &merge_with_left) {
    int remove_index = father_node_offset + 1;
    if (merge_with_left) {
      std::swap(require_node, offer_node);
      std::swap(require_node_num, offer_node_num);
      remove_index = father_node_offset;
    }
    if (require_node->is_leaf) {
      for (int i = 1; i <= offer_node->count; i++) {
        require_node->element[require_node->count + i] = offer_node->element[i];
      }
      require_node->count += offer_node->count;
      require_node->next = offer_node->next;

      // 更改父节点上的内容
      for (int i = remove_index; i < father_node->count; i++) {
        father_node->element[i] = father_node->element[i + 1];
        father_node->child[i] = father_node->child[i + 1];
      }
      father_node->count--;
    } else {
      Node tmp_child;
      int tmp_child_num;

      require_node->element[require_node->count + 1] =
          father_node->element[remove_index];
      require_node->child[require_node->count + 1] = offer_node->child[0];
      for (int i = 1; i <= offer_node->count; i++) {
        require_node->element[require_node->count + i + 1] =
            offer_node->element[i];
        require_node->child[require_node->count + i + 1] = offer_node->child[i];
      }
      require_node->count += 1 + offer_node->count;

      // 更改父节点上的内容
      for (int i = remove_index; i < father_node->count; i++) {
        father_node->element[i] = father_node->element[i + 1];
        father_node->child[i] = father_node->child[i + 1];
      }
      father_node->count--;

      // 更改子节点上的内容
      for (int i = 0; i <= offer_node->count; i++) {
        tree_node.read(tmp_child, offer_node->child[i]);
        tmp_child.fa = require_node_num;
        tree_node.update(tmp_child, offer_node->child[i]);
      }
    }
    tree_node.update(*require_node, require_node_num);

    checkremove(father_node, father_node_num);
    // tree_node.update(*father_node, father_node_num);
  }

  void checkremove(std::shared_ptr<Node> cur_node, const int &node_num) {
    // 无需对树进行调整（1. root和叶子节点重合 2. 数目保持合理）
    if ((node_num == root && cur_node->count == 0)) {
      if (!cur_node->is_leaf) {
        root = cur_node->child[0];
        Node new_root;
        tree_node.read(new_root, root);
        new_root.fa = 0;
        tree_node.update(new_root, root);
        tree_node.write_info(root, 1);
        if (new_root.is_leaf) {
          head = root;
          tree_node.write_info(head, 2);
        }
      } else {
        tree_node.update(*cur_node, node_num);
      }
      return;
    }
    int min_key = ORDER / 2;
    if ((node_num == root && cur_node->count != 0) ||
        cur_node->count >= min_key) {
      tree_node.update(*cur_node, node_num);
      return;
    }

    // 找到最大的兄弟，以判断是否可以借用
    Node father_node;
    tree_node.read(father_node, cur_node->fa);
    std::shared_ptr<Node> father_node_ptr = std::make_shared<Node>(father_node);
    // 确定在父节点哪条child分支上
    int cur_pos = -1;
    // 优先用指针定位，避免空叶节点访问 element[1]
    for (int i = 0; i <= father_node.count; i++) {
      if (father_node.child[i] == node_num) {
        cur_pos = i;
        break;
      }
    }

    if (father_node.element[father_node.count] <= cur_node->element[1]) {
      cur_pos = father_node.count;
    } else if (cur_node->element[1] < father_node.element[1]) {
      cur_pos = 0;
    } else {
      for (int i = 1; i < father_node.count; i++) {
        if (father_node.element[i] <= cur_node->element[1] &&
            cur_node->element[1] < father_node.element[i + 1]) {
          cur_pos = i;
        }
      }
    }
    Node leftbro, rightbro, maxbro;
    std::shared_ptr<Node> maxbro_ptr = std::make_shared<Node>(maxbro);
    int maxpos;
    bool can_borrow = true;
    if (cur_pos - 1 >= 0) {
      tree_node.read(leftbro, father_node.child[cur_pos - 1]);
    }
    if (cur_pos + 1 <= father_node.count) {
      tree_node.read(rightbro, father_node.child[cur_pos + 1]);
    }

    bool borrow_from_left = false;
    // 找出是否可以借，如果可以借，应该从哪边
    // 不可借
    if (leftbro.count == min_key && rightbro.count == min_key) {
      can_borrow = false;
      if (rightbro.count == 0) {
        borrow_from_left = true;
        maxbro = leftbro;
        maxpos = father_node.child[cur_pos - 1];
      } else {
        maxbro = rightbro;
        maxpos = father_node.child[cur_pos + 1];
      }
    } else {
      // 左侧借
      if (leftbro.count > rightbro.count) {
        maxbro = leftbro;
        borrow_from_left = true;
        maxpos = father_node.child[cur_pos - 1];
      } else {
        maxbro = rightbro;
        maxpos = father_node.child[cur_pos + 1];
      }
    }

    if (can_borrow) {
      borrow(cur_node, node_num, maxbro_ptr, maxpos, father_node_ptr,
             cur_node->fa, cur_pos, borrow_from_left);
    } else {
      merge(cur_node, node_num, maxbro_ptr, maxpos, father_node_ptr,
            cur_node->fa, cur_pos, borrow_from_left);
    }

    // 应该放到borrow和merge中区分才有意义
    // // 判断是否是叶子节点
    // if (cur_node->is_leaf) {
    // } else {
    // }
  }

  void remove(const index_value &target) {
    std::shared_ptr<Node> cur_node; // 先找到叶节点
    int offset = 0;
    int node_num = 0;
    findpoint(root, target, cur_node, node_num, offset);

    // std::cout << "find point check    node_size = " << cur_node->count
    //           << " the element index on offset = "
    //           << cur_node->element[offset].index
    //           << " value = " << cur_node->element[offset].value << "\n";

    // 先判断是否真的存在这个index_value
    if (cur_node->element[offset] != target) {
      return;
    }

    // 判断是否对于非叶子节点有影响
    if (offset == 1 && node_num != root) {
      // 影响了上方的节点，则找到对应的节点，然后用后继进行
      Node temp_node;
      index_value to_replace;
      if (cur_node->count == 1) {
        if (cur_node->next != 0) {
          tree_node.read(temp_node, cur_node->next);
          to_replace = temp_node.element[1];
        } else {
          to_replace = cur_node->element[1];
        }
      } else {
        to_replace = cur_node->element[2];
      }
      if (cur_node->next != 0 || cur_node->count > 1) {
        update_non_leaf_node(cur_node, node_num, cur_node->element[1],
                             to_replace);
      }
      // print_tree();
    }

    for (int i = offset; i <= cur_node->count; i++) { // 对于叶节点进行更新
      cur_node->element[i] = cur_node->element[i + 1];
    }
    cur_node->count--;

    checkremove(cur_node, node_num);
  }

  void print_tree() {
    std::cout << "\n\n---------print tree-----------\n\n";
    std::queue<int> q;
    q.push(root);
    int num = 0;
    while (!q.empty()) {
      std::cout << "Node num " << ++num << "\n";
      int cur = q.front();
      q.pop();
      Node cur_node;
      tree_node.read(cur_node, cur);
      if (cur_node.is_leaf) {
        std::cout << "is_leaf ";
      } else {
        for (int i = 0; i <= cur_node.count; i++) {
          q.push(cur_node.child[i]);
        }
        std::cout << "not_leaf ";
      }

      std::cout << "node_size = " << cur_node.count << "\n";
      for (int i = 1; i <= cur_node.count; i++) {
        std::cout << "index = " << cur_node.element[i].index
                  << " value = " << cur_node.element[i].value << "\n";
      }
    }
    std::cout << "\n\n---------print tree-----------\n\n";
  }
};

int main() {
  int oper_num;
  std::string oper;
  BPT<int> B_plus_tree;
  B_plus_tree.init();
  std::string index;
  int value;
  int ans[300005];
  std::cin >> oper_num;
  while (oper_num--) {
    std::cin >> oper;
    if (oper == "insert") {
      std::cin >> index >> value;
      BPT<int>::index_value insertation;
      for (int i = 0; i < index.length(); i++) {
        insertation.index[i] = index[i];
      }
      insertation.value = value;
      // std::cout << "start insert\n";
      B_plus_tree.insert(insertation);

      // B_plus_tree.print_tree();
    } else if (oper == "delete") {
      std::cin >> index >> value;
      BPT<int>::index_value removal;
      for (int i = 0; i < index.length(); i++) {
        removal.index[i] = index[i];
      }
      removal.value = value;
      B_plus_tree.remove(removal);
    } else if (oper == "print") {
      B_plus_tree.print_tree();
    } else {
      std::cin >> index;
      BPT<int>::index_value finding;
      for (int i = 0; i < index.length(); i++) {
        finding.index[i] = index[i];
      }
      finding.value = INT_MIN;
      int num = 0;
      bool flag = B_plus_tree.findinterval(B_plus_tree.root, finding, ans, num);
      if (!flag) {
        std::cout << "null";
      } else {
        for (int i = 1; i <= num; i++) {
          std::cout << ans[i] << " ";
        }
      }
      std::cout << std::endl;
    }
  }
  return 0;
}