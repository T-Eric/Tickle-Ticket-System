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

const int MaxSize = 4096, MinSize = 2048;  // Max=3Min
// const int MaxSize = 4, MinSize = 1;

template <class keyType, class valueType>
struct Node {
  int isLeaf = 0;  // 是否是叶结点
  int siz = 0;
  int nxt = -1;  // 如果它是叶结点，那么它的下一个块是谁（方便从底层遍历）
  // 注意指针个数会比元素个数多一个
  // 这里记录元素个数！！！！！
  Element<keyType, valueType> ele[MaxSize + 2];
  int chd[MaxSize + 3];  // 下属的块在哪个位置
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
  file.read(reinterpret_cast<char*>(&(node.isLeaf)), sizeof(node.isLeaf));
  file.read(reinterpret_cast<char*>(&(node.siz)), sizeof(node.siz));
  file.read(reinterpret_cast<char*>(&(node.nxt)), sizeof(node.nxt));
  file.read(reinterpret_cast<char*>(&(node.ele)), sizeof(node.ele));
}
GENERAL_TEMPLATE
void TREE_TYPE::wthead(int pos, NODE_TYPE& node) {
  if (pos < 0)
    throw;
  file.seekp(pos * sizeof(NODE_TYPE) + sizeof(int) * 2 + sizeof(empty));
  file.write(reinterpret_cast<const char*>(&(node.isLeaf)), sizeof(node.isLeaf));
  file.write(reinterpret_cast<const char*>(&(node.siz)), sizeof(node.siz));
  file.write(reinterpret_cast<const char*>(&(node.nxt)), sizeof(node.nxt));
  file.write(reinterpret_cast<const char*>(&(node.ele)), sizeof(node.ele));
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
  if (cur.isLeaf) {
    int l = 0, r = cur.siz;
    while (l < r) {
      int mid = (l + r) >> 1;
      if (ele < cur.ele[mid]) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    if (l > 0 && cur.ele[l - 1] == ele) {
      // 插入失败
      return false;
    }
    // 插在 l 处
    if (cur.siz < MaxSize) {
      for (int i = cur.siz - 1; i >= l; --i) {
        cur.ele[i + 1] = cur.ele[i];
      }
      ++cur.siz;
      cur.ele[l] = ele;
      wthead(pos, cur);
      // 可不可能出现要调整头顶上值的情况？
      // 貌似不会
      return false;  // 不调整
    }

    // 裂开！
    for (int i = cur.siz - 1; i >= l; --i) {
      cur.ele[i + 1] = cur.ele[i];
    }
    ++cur.siz;
    cur.ele[l] = ele;
    int newpos = GetPos();
    static Node<keyType, valueType> blk;
    blk.isLeaf = true;
    blk.siz = MinSize + 1;
    blk.nxt = cur.nxt;
    cur.nxt = newpos;
    for (int i = 0; i <= MinSize; ++i) {
      blk.ele[i] = cur.ele[i + MinSize];
    }
    cur.siz = MinSize;
    if (root == pos) {
      static Node<keyType, valueType> newroot;
      newroot.isLeaf = false;
      newroot.siz = 1;
      newroot.ele[0] = cur.ele[MinSize];
      newroot.chd[0] = pos;
      newroot.chd[1] = newpos;
      wthead(pos, cur);
      wthead(newpos, blk);
      int rootpos = GetPos();
      wt(rootpos, newroot);
      root = rootpos;
      return false;
    }
    wt(pos, cur);
    wt(newpos, blk);
    pass = blk.ele[0];
    return true;  // 调整
  }

  // 不是叶子
  int l = 0, r = cur.siz;
  while (l < r) {
    int mid = (l + r) >> 1;
    if (ele <= cur.ele[mid]) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  if (l < cur.siz && cur.ele[l] == ele) {
    ++l;
  }
  // 就是 l
  Node<keyType, valueType> child;
  rd(cur.chd[l], child);
  bool state = InternalInsert(child, cur.chd[l], ele);
  if (!state)
    return false;

  if (cur.siz < MaxSize) {
    for (int i = cur.siz - 1; i >= l; --i) {
      cur.ele[i + 1] = cur.ele[i];
      cur.chd[i + 2] = cur.chd[i + 1];
    }
    ++cur.siz;
    cur.ele[l] = pass;
    cur.chd[l + 1] = LastPos();
    wt(pos, cur);
    return false;
  }
  // 继续裂块
  for (int i = cur.siz - 1; i >= l; --i) {
    cur.ele[i + 1] = cur.ele[i];
    cur.chd[i + 2] = cur.chd[i + 1];
  }
  ++cur.siz;
  cur.ele[l] = pass;
  cur.chd[l + 1] = LastPos();
  int newpos = GetPos();
  pass = cur.ele[MinSize];
  static Node<keyType, valueType> blk;
  blk.isLeaf = false;
  blk.siz = MinSize;
  for (int i = 0; i < MinSize; ++i) {
    blk.ele[i] = cur.ele[i + MinSize + 1];
    blk.chd[i] = cur.chd[i + MinSize + 1];
  }
  blk.chd[MinSize] = cur.chd[cur.siz];
  cur.siz = MinSize;
  if (root == pos) {
    // 裂根
    static Node<keyType, valueType> newroot;
    newroot.isLeaf = false;
    newroot.siz = 1;
    newroot.ele[0] = pass;
    newroot.chd[0] = pos;
    newroot.chd[1] = newpos;
    wt(pos, cur);
    wt(newpos, blk);
    int rootpos = GetPos();
    wt(rootpos, newroot);
    root = rootpos;
    return false;
  }
  wt(pos, cur);
  wt(newpos, blk);
  return true;
}
GENERAL_TEMPLATE
bool TREE_TYPE::InternalRemove(NODE_TYPE& cur, int pos, const ELEMENT_TYPE& ele) {
  if (cur.isLeaf) {
    int l = 0, r = cur.siz;
    while (l < r) {
      int mid = (l + r) >> 1;
      if (ele < cur.ele[mid]) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    --l;
    if (l < 0 || l >= cur.siz || !(cur.ele[l] == ele)) {
      return false;
    }
    for (int i = l + 1; i < cur.siz; ++i) {
      cur.ele[i - 1] = cur.ele[i];
    }
    --cur.siz;
    if (pos == root) {
      wt(pos, cur);
    }
    wthead(pos, cur);
    if (cur.siz < MinSize) {
      return true;  // 并块
    }
    // 不用操作
    return false;
  }

  // 不是叶子
  int l = 0, r = cur.siz;
  while (l < r) {
    int mid = (l + r) >> 1;
    if (ele <= cur.ele[mid]) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  if (l < cur.siz && ele == cur.ele[l]) {
    // 刚好碰到，证明是右边的
    ++l;
  }
  // 就是 l
  Node<keyType, valueType> child;
  rd(cur.chd[l], child);
  bool state = InternalRemove(child, cur.chd[l], ele);
  if (!state)
    return false;

  // 并块！此时 child 已经删掉了一个元素，考虑跟相邻两个元素之一合并
  // 合并
  // 特判根！如果根的孩子要并块且并完只剩一个块，那么这个根消灭
  if (pos == root && cur.siz == 1) {
    static Node<keyType, valueType> blk[2];
    rdsize(cur.chd[0], blk[0]);
    rdsize(cur.chd[1], blk[1]);
    if (blk[0].siz + blk[1].siz < MaxSize) {
      // 并块！
      rd(cur.chd[0], blk[0]);
      rd(cur.chd[1], blk[1]);
      Recycle(cur.chd[1]);
      Recycle(root);
      if (blk[0].isLeaf) {
        for (int i = 0; i < blk[1].siz; ++i) {
          blk[0].ele[i + blk[0].siz] = blk[1].ele[i];
        }
        blk[0].siz += blk[1].siz;
        blk[0].nxt = blk[1].nxt;
        root = cur.chd[0];
        wthead(cur.chd[0], blk[0]);
        return false;
      }
      for (int i = 0; i < blk[1].siz; ++i) {
        blk[0].ele[i + blk[0].siz + 1] = blk[1].ele[i];
        blk[0].chd[i + blk[0].siz + 1] = blk[1].chd[i];
      }
      blk[0].chd[blk[0].siz + blk[1].siz + 1] = blk[1].chd[blk[1].siz];
      blk[0].ele[blk[0].siz] = cur.ele[0];
      blk[0].siz += blk[1].siz + 1;
      root = cur.chd[0];
      wt(cur.chd[0], blk[0]);
      return false;
    }
  }
  if (l > 0) {
    // 考虑和左边借元素 / 合并
    static Node<keyType, valueType> blk;
    rdsize(cur.chd[l - 1], blk);
    if (blk.siz > MinSize) {
      // 从左边借一个
      if (child.isLeaf) {
        rdhead(cur.chd[l - 1], blk);
        for (int i = child.siz - 1; i >= 0; --i) {
          child.ele[i + 1] = child.ele[i];
        }
        child.ele[0] = blk.ele[blk.siz - 1];
        ++child.siz;
        --blk.siz;
        cur.ele[l - 1] = child.ele[0];
        wt(pos, cur);
        wtsize(cur.chd[l - 1], blk);
        wthead(cur.chd[l], child);
        return false;
      }
      rd(cur.chd[l - 1], blk);
      for (int i = child.siz; i >= 1; --i) {
        child.ele[i] = child.ele[i - 1];
        child.chd[i + 1] = child.chd[i];
      }
      child.chd[1] = child.chd[0];
      ++child.siz;
      child.ele[0] = cur.ele[l - 1];
      child.chd[0] = blk.chd[blk.siz];
      cur.ele[l - 1] = blk.ele[blk.siz - 1];
      --blk.siz;
      wt(pos, cur);
      wtsize(cur.chd[l - 1], blk);
      wt(cur.chd[l], child);
      return false;
    }
    // 和左边合并
    if (child.isLeaf) {
      rdhead(cur.chd[l - 1], blk);
      Recycle(cur.chd[l]);
      for (int i = 0; i < child.siz; ++i) {
        blk.ele[i + blk.siz] = child.ele[i];
      }
      blk.siz += child.siz;
      blk.nxt = child.nxt;
      for (int i = l; i < cur.siz; ++i) {
        cur.ele[i - 1] = cur.ele[i];
        cur.chd[i] = cur.chd[i + 1];
      }
      --cur.siz;
      blk.nxt = child.nxt;
      wt(pos, cur);
      wthead(cur.chd[l - 1], blk);
      if (cur.siz < MinSize)
        return true;
      return false;
    }
    rd(cur.chd[l - 1], blk);
    Recycle(cur.chd[l]);
    for (int i = 0; i < child.siz; ++i) {
      blk.ele[i + blk.siz + 1] = child.ele[i];
      blk.chd[i + blk.siz + 1] = child.chd[i];
    }
    blk.chd[blk.siz + child.siz + 1] = child.chd[child.siz];
    blk.ele[blk.siz] = cur.ele[l - 1];
    blk.siz += child.siz + 1;
    for (int i = l - 1; i < cur.siz - 1; ++i) {
      cur.ele[i] = cur.ele[i + 1];
      cur.chd[i + 1] = cur.chd[i + 2];
    }
    --cur.siz;
    wt(pos, cur);
    wt(cur.chd[l - 1], blk);
    if (cur.siz < MinSize)
      return true;
    return false;
  } else if (l < cur.siz) {
    // 和右边借元素 / 合并
    static Node<keyType, valueType> blk;
    rdsize(cur.chd[l + 1], blk);
    if (blk.siz > MinSize) {
      // 从右边借一个
      if (child.isLeaf) {
        rdhead(cur.chd[l + 1], blk);
        child.ele[child.siz] = blk.ele[0];
        ++child.siz;
        for (int i = 0; i < blk.siz - 1; ++i) {
          blk.ele[i] = blk.ele[i + 1];
        }
        --blk.siz;
        cur.ele[l] = blk.ele[0];
        wt(pos, cur);
        wthead(cur.chd[l], child);
        wthead(cur.chd[l + 1], blk);
        return false;
      }
      rd(cur.chd[l + 1], blk);
      child.ele[child.siz] = cur.ele[l];
      child.chd[child.siz + 1] = blk.chd[0];
      ++child.siz;
      cur.ele[l] = blk.ele[0];
      for (int i = 0; i < blk.siz - 1; ++i) {
        blk.ele[i] = blk.ele[i + 1];
        blk.chd[i] = blk.chd[i + 1];
      }
      blk.chd[blk.siz - 1] = blk.chd[blk.siz];
      --blk.siz;
      wt(pos, cur);
      wt(cur.chd[l], child);
      wt(cur.chd[l + 1], blk);
      return false;
    }
    // 和右边合并
    if (child.isLeaf) {
      rdhead(cur.chd[l + 1], blk);
      Recycle(cur.chd[l + 1]);
      for (int i = 0; i < blk.siz; ++i) {
        child.ele[i + child.siz] = blk.ele[i];
      }
      child.siz += blk.siz;
      child.nxt = blk.nxt;
      for (int i = l; i < cur.siz - 1; ++i) {
        cur.ele[i] = cur.ele[i + 1];
        cur.chd[i + 1] = cur.chd[i + 2];
      }
      --cur.siz;
      child.nxt = blk.nxt;
      wt(pos, cur);
      wthead(cur.chd[l], child);
      if (cur.siz < MinSize)
        return true;
      return false;
    }
    rd(cur.chd[l + 1], blk);
    Recycle(cur.chd[l + 1]);
    for (int i = 0; i < blk.siz; ++i) {
      child.ele[i + child.siz + 1] = blk.ele[i];
      child.chd[i + child.siz + 1] = blk.chd[i];
    }
    child.chd[child.siz + blk.siz + 1] = blk.chd[blk.siz];
    child.ele[child.siz] = cur.ele[l];
    child.siz += blk.siz + 1;
    for (int i = l; i < cur.siz - 1; ++i) {
      cur.ele[i] = cur.ele[i + 1];
      cur.chd[i + 1] = cur.chd[i + 2];
    }
    --cur.siz;
    wt(pos, cur);
    wt(cur.chd[l], child);
    if (cur.siz < MinSize)
      return true;
    return false;
  } else {
    exit(1);
  }
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
  cur.isLeaf = false;
  int pos = root, mid;
  while (true) {
    rd(pos, cur);
    if (cur.isLeaf)
      break;
    int l = 0, r = cur.siz;
    while (l < r) {
      mid = (l + r) >> 1;
      if (key <= cur.ele[mid].key)
        r = mid;
      else
        l = mid + 1;
    }
    pos = cur.chd[l];
  }
  int l = 0, r = cur.siz;
  while (l < r) {
    mid = (l + r) >> 1;
    if (key <= cur.ele[mid].key)
      r = mid;
    else
      l = mid + 1;
  }
  if (l > 0)
    --l;
  if (l < cur.siz && key < cur.ele[l].key)
    return;
  bool flag = false;
  while (true) {
    for (int i = l; i < cur.siz; ++i) {
      if (key < cur.ele[i].key) {
        flag = true;
        break;
      }
      if (key == cur.ele[i].key)
        ans.push_back(cur.ele[i].val);
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
    cur.ele[0] = ele;
    cur.isLeaf = true;
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
  if (root = -1)
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