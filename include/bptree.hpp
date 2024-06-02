#ifndef SJTU_BPTREE_HPP
#define SJTU_BPTREE_HPP

#include "utils.hpp"

#define GENERAL_TEMPLATE template <class keyType, class valueType>
#define ELEMENT_TYPE Element<keyType, valueType>
#define NODE_TYPE Node<keyType, valueType>
#define TREE_TYPE BPTree<keyType, valueType>

using std::cin;
using std::cout;
using std::string;

namespace sjtu {

template <class keyType, class valueType>
struct Element {
  keyType key;
  valueType val;

  Element() = default;
  Element(const keyType& k, const valueType& v);
  Element(const Element& other);
  Element& operator=(const Element& other);
  ~Element() = default;
  template <class K, class V>
  friend bool operator==(const Element<K, V>& lhs, const Element<K, V>& rhs);
  template <class K, class V>
  friend bool operator<(const Element<K, V>& lhs, const Element<K, V>& rhs);
  template <class K, class V>
  friend bool operator<=(const Element<K, V>& lhs, const Element<K, V>& rhs);
};

const int MaxSize = 4096, MinSize = 1365;  // Max=3Min
// const int MaxSize = 4, MinSize = 1;

template <class keyType, class valueType>
struct Node {
  int isleaf = 0;
  int siz = 0;
  int nxt = -1;
  ELEMENT_TYPE eles[MaxSize + 2];
  // head_part
  int chds[MaxSize + 3];  // 如果是叶节点，就指向data了
  // 但这样就要搞data回收，恐怕十分困难吧
};

template <class keyType, class valueType>
class BPTree {
 private:
  int nowsize = -1;  // 最后一个块的位置
  std::fstream file;
  std::string filename;
  int empty[1001];  // empty[0]就能存放栈高度了
  int last = -1;
  ELEMENT_TYPE pass;  // 中间的元素

  void rd(int pos, NODE_TYPE& node);
  void wt(int pos, NODE_TYPE& node);
  void rdhead(int pos, NODE_TYPE& node);
  void wthead(int pos, NODE_TYPE& node);
  void rdsize(int pos, NODE_TYPE& node);
  void wtsize(int pos, NODE_TYPE& node);

  int GetPos();                  // 获取一个空白块
  inline int LastPos() const;    // 求上一个
  inline void Recycle(int pos);  // 扔到回收站

  bool InternalInsert(NODE_TYPE& cur, int pos, const ELEMENT_TYPE& ele);
  bool InternalRemove(NODE_TYPE& cur, int pos, const ELEMENT_TYPE& ele);

 public:
  int root = -1;  // 根节点pos，必须第一时间获取
  explicit BPTree(const std::string& name);
  ~BPTree();

  void Find(const keyType& key, sjtu::vector<valueType>& ans);
  void Insert(const ELEMENT_TYPE& ele);
  void Remove(const ELEMENT_TYPE& ele);
  void Clear();
  void BulkFind(const keyType& lkey, const keyType& rkey, sjtu::vector<ELEMENT_TYPE>& ans);
};

GENERAL_TEMPLATE
ELEMENT_TYPE::Element(const keyType& k, const valueType& v) {
  key = k;
  val = v;
}
GENERAL_TEMPLATE
ELEMENT_TYPE::Element(const Element& other) {
  key = other.key;
  val = other.val;
}
GENERAL_TEMPLATE
ELEMENT_TYPE& ELEMENT_TYPE::operator=(const Element& other) {
  if (this != &other) {
    key = other.key;
    val = other.val;
  }
  return *this;
}
GENERAL_TEMPLATE
bool operator==(const ELEMENT_TYPE& lhs, const ELEMENT_TYPE& rhs) {
  if (!(lhs.key == rhs.key))
    return false;
  return lhs.val == rhs.val;
}
GENERAL_TEMPLATE
bool operator<(const ELEMENT_TYPE& lhs, const ELEMENT_TYPE& rhs) {
  if (!(lhs.key == rhs.key))
    return lhs.key < rhs.key;
  return lhs.val < rhs.val;
}
GENERAL_TEMPLATE
bool operator<=(const ELEMENT_TYPE& lhs, const ELEMENT_TYPE& rhs) {
  return lhs == rhs || lhs < rhs;
}

GENERAL_TEMPLATE
void TREE_TYPE::rd(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekg(pos * sizeof(NODE_TYPE) + sizeof(int) * 2 + sizeof(empty));
  file.read(reinterpret_cast<char*>(&node), sizeof(node));
}
GENERAL_TEMPLATE
void TREE_TYPE::wt(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekp(pos * sizeof(NODE_TYPE) + sizeof(int) * 2 + sizeof(empty));
  file.write(reinterpret_cast<const char*>(&node), sizeof(node));
}
GENERAL_TEMPLATE
void TREE_TYPE::rdhead(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekg(pos * sizeof(NODE_TYPE) + sizeof(int) * 2 + sizeof(empty));
  file.read(reinterpret_cast<char*>(&(node.isleaf)), sizeof(node.isleaf));
  file.read(reinterpret_cast<char*>(&(node.siz)), sizeof(node.siz));
  file.read(reinterpret_cast<char*>(&(node.nxt)), sizeof(node.nxt));
  file.read(reinterpret_cast<char*>(&(node.eles)), sizeof(node.eles));
}
GENERAL_TEMPLATE
void TREE_TYPE::wthead(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekp(pos * sizeof(NODE_TYPE) + sizeof(int) * 2 + sizeof(empty));
  file.write(reinterpret_cast<const char*>(&(node.isleaf)), sizeof(node.isleaf));
  file.write(reinterpret_cast<const char*>(&(node.siz)), sizeof(node.siz));
  file.write(reinterpret_cast<const char*>(&(node.nxt)), sizeof(node.nxt));
  file.write(reinterpret_cast<const char*>(&(node.eles)), sizeof(node.eles));
}
GENERAL_TEMPLATE
void TREE_TYPE::rdsize(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekg(pos * sizeof(NODE_TYPE) + sizeof(int) * 3 + sizeof(empty));
  file.read(reinterpret_cast<char*>(&(node.siz)), sizeof(node.siz));
}
GENERAL_TEMPLATE
void TREE_TYPE::wtsize(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekp(pos * sizeof(NODE_TYPE) + sizeof(int) * 3 + sizeof(empty));
  file.write(reinterpret_cast<const char*>(&(node.siz)), sizeof(node.siz));
}

GENERAL_TEMPLATE
int TREE_TYPE::GetPos() {
  if (empty[0]) {
    int ret = empty[empty[0]--];
    last = ret;  // 记录上一个last块
    return ret;
  }
  last = ++nowsize;  // 为什么要++呢？
  return nowsize;
}
GENERAL_TEMPLATE
inline int TREE_TYPE::LastPos() const {
  return last;
}
GENERAL_TEMPLATE
inline void TREE_TYPE::Recycle(int pos) {
  empty[++empty[0]] = pos;
}

GENERAL_TEMPLATE
bool TREE_TYPE::InternalInsert(NODE_TYPE& cur, int pos, const ELEMENT_TYPE& ele) {
  if (cur.isleaf) {
    int l = 0, r = cur.siz, mid;
    while (l < r) {
      mid = (l + r) >> 1;
      if (ele < cur.eles[mid])
        r = mid;
      else
        l = mid + 1;
    }
    if (l > 0 && cur.eles[l - 1] == ele)
      return false;  // 相同元素

    for (int i = cur.siz - 1; i >= l; --i)
      cur.eles[i + 1] = cur.eles[i];
    cur.eles[l] = ele;  // 先插入
    ++cur.siz;
    if (cur.siz <= MaxSize) {
      wthead(pos, cur);
      return false;  // 没有裂块，不必调整
    }
    // 裂块
    else {
      int newpos = GetPos();
      static NODE_TYPE node;               // 常驻新node
      int rightsize = (MaxSize >> 1) + 1;  // 裂成相同大小的块就得
      // rightsize=257，cur.siz=513
      node.isleaf = true;
      node.siz = rightsize;
      node.nxt = cur.nxt;
      cur.nxt = newpos;
      for (int i = 0; i < rightsize; ++i)
        node.eles[i] = cur.eles[i + rightsize - 1];
      // 假如根节点要裂开
      if (root == pos) {
        // 建立新的根节点
        static NODE_TYPE newroot;
        newroot.isleaf = false;
        newroot.siz = 1;
        newroot.eles[0] = cur.eles[rightsize - 1];
        newroot.chds[0] = pos;
        newroot.chds[1] = newpos;
        wthead(pos, cur);
        wthead(newpos, node);
        int rootpos = GetPos();
        wt(rootpos, newroot);
        root = rootpos;
        return false;
      }
      wt(pos, cur);
      wt(newpos, node);
      pass = node.eles[0];  // 上传的新节点值
      return true;
    }
  }
  // 内部节点
  int l = 0, r = cur.siz, mid;
  while (l < r) {
    mid = (l + r) >> 1;
    if (ele <= cur.eles[mid])
      r = mid;
    else
      l = mid + 1;
  }
  if (l < cur.siz && cur.eles[l] == ele)
    ++l;
  NODE_TYPE child;
  rd(cur.chds[l], child);
  // 递归插入
  bool state = InternalInsert(child, cur.chds[l], ele);
  if (!state)
    return false;  // 不调整

  for (int i = cur.siz - 1; i >= l; --i) {
    cur.eles[i + 1] = cur.eles[i];
    cur.chds[i + 2] = cur.chds[i + 1];
  }
  ++cur.siz;
  cur.eles[l] = pass;
  cur.chds[l + 1] = LastPos();
  if (cur.siz <= MaxSize) {
    wt(pos, cur);
    return false;
  }
  int newpos = GetPos(), rightsize = (MaxSize >> 1) + 1;
  pass = cur.eles[rightsize - 1];
  static NODE_TYPE node;
  node.isleaf = false;
  node.siz = rightsize;
  for (int i = 0; i < rightsize - 1; ++i) {
    node.eles[i] = cur.eles[i + rightsize];
    node.chds[i] = cur.chds[i + rightsize];
  }
  node.chds[rightsize - 1] = cur.chds[cur.siz];
  cur.siz = rightsize - 1;
  if (root == pos) {
    static NODE_TYPE newroot;
    newroot.isleaf = false;
    newroot.siz = 1;
    newroot.eles[0] = pass;
    newroot.chds[0] = pos;
    newroot.chds[1] = newpos;
    wt(pos, cur);
    wt(newpos, node);
    int rootpos = GetPos();
    wt(rootpos, newroot);
    root = rootpos;
    return false;
  }
  wt(pos, cur);
  wt(newpos, node);
  return true;
}
GENERAL_TEMPLATE
bool TREE_TYPE::InternalRemove(NODE_TYPE& cur, int pos, const ELEMENT_TYPE& ele) {
  if (cur.isleaf) {
    int l = 0, r = cur.siz, mid;
    while (l < r) {
      mid = (l + r) >> 1;
      if (ele < cur.eles[mid])
        r = mid;
      else
        l = mid + 1;
    }
    --l;
    if (l < 0 || l >= cur.siz || !(cur.eles[l] == ele))
      return false;
    for (int i = l + 1; i < cur.siz; ++i)
      cur.eles[i - 1] = cur.eles[i];
    --cur.siz;
    if (pos == root)
      wt(pos, cur);
    wthead(pos, cur);
    if (cur.siz < MinSize)
      return true;
    return false;
  }
  int l = 0, r = cur.siz;
  while (l < r) {
    int mid = (l + r) >> 1;
    if (ele <= cur.eles[mid]) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  if (l < cur.siz && ele == cur.eles[l])
    ++l;
  NODE_TYPE child;
  rd(cur.chds[l], child);
  bool state = InternalRemove(child, cur.chds[l], ele);
  if (!state)
    return false;
  // 特判根！如果根的孩子要并块且并完只剩一个块，那么这个根消灭
  if (pos == root && cur.siz == 1) {
    static NODE_TYPE node[2];
    rdsize(cur.chds[0], node[0]);
    rdsize(cur.chds[1], node[1]);
    if (node[0].siz + node[1].siz < MaxSize) {
      rd(cur.chds[0], node[0]);
      rd(cur.chds[1], node[1]);
      Recycle(cur.chds[1]);
      Recycle(root);
      if (node[0].isleaf) {
        for (int i = 0; i < node[1].siz; ++i)
          node[0].eles[i + node[0].siz] = node[1].eles[i];
        node[0].siz += node[1].siz;
        node[0].nxt = node[1].nxt;
        root = cur.chds[0];
        wthead(cur.chds[0], node[0]);
        return false;
      }
      for (int i = 0; i < node[1].siz; ++i) {
        node[0].eles[i + node[0].siz + 1] = node[1].eles[i];
        node[0].chds[i + node[0].siz + 1] = node[1].chds[i];
      }
      node[0].chds[node[0].siz + node[1].siz + 1] = node[1].chds[node[1].siz];
      node[0].eles[node[0].siz] = cur.eles[0];
      node[0].siz += node[1].siz + 1;
      root = cur.chds[0];
      wt(cur.chds[0], node[0]);
      return false;
    }
  }
  if (l > 0) {
    // 考虑和左边借元素或者合并
    static NODE_TYPE node;
    rdsize(cur.chds[l - 1], node);
    if (node.siz > MinSize) {
      if (child.isleaf) {
        rdhead(cur.chds[l - 1], node);
        for (int i = child.siz - 1; i >= 0; --i)
          child.eles[i + 1] = child.eles[i];
        child.eles[0] = node.eles[node.siz - 1];
        ++child.siz;
        --node.siz;
        cur.eles[l - 1] = child.eles[0];
        wt(pos, cur);
        wtsize(cur.chds[l - 1], node);
        wthead(cur.chds[l], child);
        return false;
      }
      rd(cur.chds[l - 1], node);
      for (int i = child.siz; i >= 1; --i) {
        child.eles[i] = child.eles[i - 1];
        child.chds[i + 1] = child.chds[i];
      }
      child.chds[1] = child.chds[0];
      child.eles[0] = cur.eles[l - 1];
      child.chds[0] = node.chds[node.siz];
      ++child.siz;
      cur.eles[l - 1] = node.eles[node.siz - 1];
      --node.siz;
      wt(pos, cur);
      wtsize(cur.chds[l - 1], node);
      wt(cur.chds[l], child);
      return false;
    }
    // 和左边合并
    if (child.isleaf) {
      rdhead(cur.chds[l - 1], node);
      Recycle(cur.chds[l - 1]);
      for (int i = 0; i < child.siz; ++i)
        node.eles[i + node.siz] = child.eles[i];
      node.siz += child.siz;
      node.nxt = child.nxt;
      for (int i = l; i < cur.siz; ++i) {
        cur.eles[i - 1] = cur.eles[i];
        cur.chds[i] = cur.chds[i + 1];
      }
      --cur.siz;
      wt(cur.chds[l - 1], node);
      wthead(cur.chds[l - 1], node);
      if (cur.siz < MinSize)
        return true;
      return false;
    }
    rd(cur.chds[l - 1], node);
    Recycle(cur.chds[l]);
    for (int i = 0; i < child.siz; ++i) {
      node.eles[i + node.siz + 1] = child.eles[i];
      node.chds[i + node.siz + 1] = child.chds[i];
    }
    node.chds[node.siz + child.siz + 1] = child.chds[child.siz];
    node.eles[node.siz] = cur.eles[l - 1];
    node.siz += child.siz + 1;
    for (int i = l; i < cur.siz; ++i) {
      cur.eles[i - 1] = cur.eles[i];
      cur.chds[i] = cur.chds[i + 1];
    }
    --cur.siz;
    wt(pos, cur);
    wt(cur.chds[l - 1], node);
    if (cur.siz < MinSize)
      return true;
    return false;
  } else if (l < cur.siz) {
    // 右边借/并
    static NODE_TYPE node;
    rdsize(cur.chds[l + 1], node);
    if (node.siz > MinSize) {
      if (child.isleaf) {
        rdhead(cur.chds[l + 1], node);
        child.eles[child.siz] = node.eles[0];
        ++child.siz;
        for (int i = 0; i < node.siz - 1; ++i)
          node.eles[i] = node.eles[i + 1];
        --node.siz;
        cur.eles[l] = node.eles[0];
        wt(pos, cur);
        wthead(cur.chds[l + 1], node);
        wthead(cur.chds[l + 1], node);
        return false;
      }
      rd(cur.chds[l + 1], node);
      child.eles[child.siz] = cur.eles[l];
      child.chds[child.siz + 1] = node.chds[0];
      ++child.siz;
      cur.eles[l] = node.eles[0];
      for (int i = 0; i < node.siz - 1; ++i) {
        node.eles[i] = node.eles[i + 1];
        node.chds[i] = node.chds[i + 1];
      }
      node.chds[node.siz - 1] = node.chds[node.siz];
      --node.siz;
      wt(pos, cur);
      wt(cur.chds[l], child);
      wt(cur.chds[l + 1], node);
      return false;
    }
    // 右合并
    if (child.isleaf) {
      rdhead(cur.chds[l + 1], node);
      Recycle(cur.chds[l + 1]);
      for (int i = 0; i < node.siz; ++i)
        child.eles[i + child.siz] = node.eles[i];
      child.siz += node.siz;
      child.nxt = node.nxt;
      for (int i = l + 1; i < cur.siz; ++i) {
        cur.eles[i - 1] = cur.eles[i];
        cur.chds[i] = cur.chds[i + 1];
      }
      --cur.siz;
      wt(pos, cur);
      wt(cur.chds[l], child);
      if (cur.siz < MinSize)
        return true;
      return false;
    }
    rd(cur.chds[l + 1], node);
    ;
    Recycle(cur.chds[l + 1]);
    for (int i = 0; i < node.siz; ++i) {
      child.eles[i + child.siz + 1] = node.eles[i];
      child.chds[i + child.siz + 1] = node.chds[i];
    }
    child.chds[child.siz + node.siz + 1] = node.chds[node.siz];
    child.eles[child.siz] = cur.eles[l];
    child.siz += node.siz + 1;
    for (int i = l; i < cur.siz - 1; ++i) {
      cur.eles[i] = cur.eles[i + 1];
      cur.chds[i + 1] = cur.chds[i + 2];
    }
    --cur.siz;
    wt(pos, cur);
    wt(cur.chds[l], child);
    if (cur.siz < MinSize)
      return true;
    return false;
  } else
    throw;
}

GENERAL_TEMPLATE
TREE_TYPE::BPTree(const std::string& name) {
  filename = name;
  file.open(filename);
  if (!file.is_open()) {
    file.open(filename, std::fstream::out);
    file.close();
    file.open(filename, std::fstream::in | std::fstream::out);
    nowsize = -1;
    file.seekp(0);
    file.write(reinterpret_cast<const char*>(&nowsize), sizeof(nowsize));
    root = -1;
    file.write(reinterpret_cast<const char*>(&root), sizeof(root));
    empty[0] = 0;
    file.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
    static NODE_TYPE tmpnode;
    file.write(reinterpret_cast<const char*>(&tmpnode), sizeof(tmpnode));
  } else {
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&nowsize), sizeof(nowsize));
    file.read(reinterpret_cast<char*>(&root), sizeof(root));
    file.read(reinterpret_cast<char*>(&empty), sizeof(empty));
  }
}
GENERAL_TEMPLATE
TREE_TYPE::~BPTree() {
  file.seekp(0);
  file.write(reinterpret_cast<const char*>(&nowsize), sizeof(nowsize));
  file.write(reinterpret_cast<const char*>(&root), sizeof(root));
  file.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
  file.close();
}
GENERAL_TEMPLATE
void TREE_TYPE::Find(const keyType& key, sjtu::vector<valueType>& ans) {
  ans.clear();
  if (root == -1)
    return;
  static NODE_TYPE cur;
  cur.isleaf = false;
  int pos = root, mid;
  while (true) {
    rd(pos, cur);
    if (cur.isleaf)
      break;
    int l = 0, r = cur.siz;
    while (l < r) {
      mid = (l + r) >> 1;
      if (key <= cur.eles[mid].key)
        r = mid;
      else
        l = mid + 1;
    }
    pos = cur.chds[l];
  }
  int l = 0, r = cur.siz;
  while (l < r) {
    mid = (l + r) >> 1;
    if (key <= cur.eles[mid].key)
      r = mid;
    else
      l = mid + 1;
  }
  if (l > 0)
    --l;
  if (l < cur.siz && key < cur.eles[l].key)
    return;
  bool flag = false;
  while (true) {
    for (int i = l; i < cur.siz; ++i) {
      if (key < cur.eles[i].key) {
        flag = true;
        break;
      }
      if (key == cur.eles[i].key)
        ans.push_back(cur.eles[i].val);
    }
    if (flag)
      break;
    pos = cur.nxt;
    if (pos == -1)
      break;
    rdhead(pos, cur);
    l = 0;
  }
  return;
}
GENERAL_TEMPLATE
void TREE_TYPE::Insert(const ELEMENT_TYPE& ele) {
  if (root == -1) {
    root = nowsize = 0;
    static NODE_TYPE cur;
    cur.siz = 1;
    cur.eles[0] = ele;
    cur.isleaf = true;
    cur.nxt = -1;
    wthead(nowsize, cur);
    return;
  }
  NODE_TYPE cur;
  rd(root, cur);
  InternalInsert(cur, root, ele);
}
GENERAL_TEMPLATE
void TREE_TYPE::Remove(const ELEMENT_TYPE& ele) {
  if (root == -1)
    return;
  static NODE_TYPE cur;
  rd(root, cur);
  InternalRemove(cur, root, ele);
}
GENERAL_TEMPLATE
void TREE_TYPE::Clear() {
  if (std::filesystem::exists(filename))
    std::filesystem::remove(filename);
  file.open(filename, std::fstream::out);
  file.close();
  file.open(filename, std::fstream::in | std::fstream::out);
  nowsize = -1;
  file.seekp(0);
  file.write(reinterpret_cast<const char*>(&nowsize), sizeof(nowsize));
  root = -1;
  file.write(reinterpret_cast<const char*>(&root), sizeof(root));
  empty[0] = 0;
  file.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
  static NODE_TYPE tmpnode;
  file.write(reinterpret_cast<const char*>(&tmpnode), sizeof(tmpnode));
}
GENERAL_TEMPLATE
void TREE_TYPE::BulkFind(const keyType& lkey, const keyType& rkey, sjtu::vector<ELEMENT_TYPE>& ans) {
  return;
}

}  // namespace sjtu

#endif  // !SJTU_BPTREE_HPP