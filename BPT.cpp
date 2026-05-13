#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

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
    file.open(file_name, std::ios::out);
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
    file.seekg(n - 1, std::ios::beg);
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
  }

  // 将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len)
      return;
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(n - 1, std::ios::beg);
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
  static const int ORDER = 128;
  struct index_value {
    char index[64] = {};
    T value = T();

    bool operator<(const index_value &other) {
      int cmp = strcmp(this->index, other.index);
      if (cmp < 0)
        return true;
      else if (cmp > 0)
        return false;
      else {
        return value < other.value;
      }
    }
  };
  struct Node {
    int fa;
    int child[ORDER + 5]; // 0-127
    size_t count;
    index_value element[ORDER + 5];
    // 1-127 这里预留空间+5保证不会split之前超出限制
    bool is_leaf;
    int next;

    Node() {}
  };

  MemoryRiver<Node> tree_node = ("tree_node");

public:
  int head;
  int root;
  bool findinterval(const int &pos, const index_value &target, int &offset,
                    int *vec, int &num) {
    Node cur_node;
    tree_node.read(cur_node, pos);
    offset = 0;
    num = 0;
    if (cur_node.is_leaf) {
      for (int i = 1; i <= cur_node.count; i++) {
        if (strcmp(target.index, cur_node.element[i].index) == 0) {
          offset = i;
          break;
        }
      }
      if (offset == 0) { // 有可能是下一组的第一个
        int next_index = cur_node.next;
        tree_node.read(cur_node, next_index);
        if (strcmp(target.index, cur_node.element[1].index) == 0) {
          offset = 1;
        }
      }
      if (offset != 0) { // 确实存在这个index
        for (int i = offset; i <= cur_node.count; i++) {
          // 当前element index仍然和搜索的要求一致
          if (cur_node.element[i].index == target.index) {
            vec[++num] = cur_node.element[i].value;
            // 需要继续进入下一个块读入
            if (i == cur_node.count) {
              tree_node.read(cur_node, cur_node.next);
            }
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
    if (strcmp(target.index, cur_node.element[1].index) < 0) {
      return findinterval(cur_node.child[0], target, offset, vec, num);
    }
    for (int i = 1; i < cur_node.count; i++) {
      if (strcmp(target.index, cur_node.element[i].index) >= 0 &&
          strcmp(target.index, cur_node.element[i + 1].index) < 0) {
        return findinterval(cur_node.child[i], target, offset, vec, num);
      }
    }
    return findinterval(cur_node.child[cur_node.count], target, offset, vec,
                        num);
  }

  void findpoint(const int &pos, const index_value &target, Node *out_node,
                 int &node_num, int &offset) {
    Node cur_node;
    tree_node.read(cur_node, pos);

    if (cur_node.is_leaf) {
      out_node = &cur_node;
      node_num = pos;
      if (target < cur_node.element[1]) {
        offset = 1;
        return;
      }
      for (int i = 1; i < cur_node.count; i++) {
        if (target > cur_node.element[i] && target < cur_node.element[i + 1]) {
          offset = i + 1;
          return;
        }
      }
      offset = cur_node.count + 1;
      return;
    }

    if (strcmp(target.index, cur_node.element[1].index) < 0) {
      findpoint(cur_node.child[0], target, out_node, offset);
      return;
    }
    for (int i = 1; i < cur_node.count; i++) {
      if (strcmp(target.index, cur_node.element[i].index) >= 0 &&
          strcmp(target.index, cur_node.element[i + 1].index) < 0) {
        findpoint(cur_node.child[i], target, out_node, offset);
        return;
      }
    }
    findpoint(cur_node.child[cur_node.count], target, out_node, offset);
  }

  void split(Node *cur_node, const int &node_num) {
    // 如果是根节点，则申请空的节点，然后其他依旧保持一致
    if (node_num == root) {
      Node tmp_fa;
      cur_node->fa = tree_node.write(tmp_fa);
      root = cur_node->fa;
    }

    index_value min_of_child = cur_node->element[1];
    Node father_node;
    tree_node.read(father_node, cur_node->fa);

    int father_offset;
    if (min_of_child < father_node.element[1]) {
      father_offset = 0;
    } else {
      for (int i = 1; i <= father_node.count; i++) {
        if (father_node.element[i] <= min_of_child) {
          father_offset = i;
          break;
        }
      }
    }

    int selected_offset = ceil((double)ORDER / 2.0);

    for (int i = father_node.count; i >= father_offset; i--) {
      father_node.element[i + 1] = father_node.element[i];
      father_node.child[i + 1] = father_node.child[i];
    }
    father_node.element[father_offset] = cur_node->element[selected_offset];
    father_node.count++;

    Node tmp;
    tmp.fa = cur_node->fa;
    for (int i = selected_offset; i <= cur_node->count; i++) {
      tmp.child[i - selected_offset] = cur_node->child[i];
    }
    tmp.count = cur_node->count - selected_offset;
    for (int i = selected_offset + 1; i <= cur_node->count; i++) {
      tmp.child[i - selected_offset] = cur_node->child[i];
    }
    tmp.is_leaf = cur_node->is_leaf;
    if (tmp.is_leaf) {
      tmp.next = cur_node->next;
    } else {
      tmp.next = nullptr;
    }
    int new_address = tree_node.write(*tmp);
    cur_node->next = new_address;
    tree_node.update(cur_node, node_num);

    father_node.child[father_offset] = new_address;
    for (int i = selected_offset; i <= cur_node->count; i++) {
      Node tmp_child;
      tree_node.read(tmp_child, tmp.child[i]);
      tmp_child.fa = new_address;
      tree_node.update(tmp_child, tmp.child[i]);
    }

    cur_node->count = selected_offset - 1;

    checkinsert(&father_node, cur_node->fa);
  }

  void checkinsert(Node *cur_node, const int &node_num) {
    if (cur_node->count <= ORDER - 1) { // 没有问题
      tree_node.update(cur_node, node_num);
      return;
    }
    if (node_num == root) { // 如果是root还需要特殊处理
    }
    split(cur_node, node_num);
  }

  void insert(const index_value &target) {
    // step1 找位置
    Node *cur_node; // 先找到叶节点
    int offset = 0;
    int node_num = 0;
    findpoint(root, target, cur_node, node_num, offset);

    // step2 内存中更新
    for (int i = cur_node->count; i >= offset; i--) {
      cur_node->element[i + 1] = cur_node->element[i];
    }
    cur_node->element[offset] = target;
    cur_node->count++;

    // step3 检查是否上溢，如果没有则写入，如果有，进行相应调整
    checkinsert(cur_node, node_num);
  }
};

int main() { return 0; }