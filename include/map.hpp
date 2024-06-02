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

template <
    class Key,
    class T,
    class Compare = std::less<Key> >
class map {
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;
  class iterator;
  class const_iterator;

 private:
  friend class iterator;
  friend class const_iterator;
  enum Color {
    BLACK,
    RED
  };
  enum Direction {
    LEFT = -1,
    ROOT = 0,
    RIGHT = 1
  };
  Compare compare = Compare();
  struct Node {
    Color color = RED;
    Node *parent = nullptr, *left = nullptr, *right = nullptr;
    value_type value;

    explicit Node(const value_type& _value)
        : value(_value) {}
    explicit Node(Node* other)
        : value(other->value) {
      color = other->color;
    }
    ~Node() = default;
    inline Direction direction() const {
      if (parent == nullptr)
        return Direction::ROOT;
      if (this == parent->left)
        return Direction::LEFT;
      return Direction::RIGHT;
    }
    inline bool isLeaf() {
      return left == nullptr && right == nullptr;
    }
    inline Node* uncle() {
      if (parent == nullptr || parent->parent == nullptr)
        return nullptr;
      return parent->parent->left == parent ? parent->parent->right : parent->parent->left;
    }
    inline Node* grandparent() {
      if (parent == nullptr)
        return nullptr;
      return parent->parent;
    }
    inline Node* sibling() {
      if (parent == nullptr)
        return nullptr;
      return parent->left == this ? parent->right : parent->left;
    }
  };
  Node* root;
  size_t siz = 0;

  Node* stack[32];
  Node* ostack[32];
  int top = -1, otop = -1;
  //----------------------------------------------------------------
  void maintainRelationship(Node* node) {
    if (node == nullptr)
      return;
    if (node->left != nullptr)
      node->left->parent = node;
    if (node->right != nullptr)
      node->right->parent = node;
  }
  // rotate
  void rotateLeft(Node* node) {
    if (node == nullptr || node->right == nullptr)
      return;
    Node* par = node->parent;
    Direction dir = node->direction();
    Node* suc = node->right;
    node->right = suc->left;
    suc->left = node;
    maintainRelationship(node);
    maintainRelationship(suc);  // maintain the sons of node and suc
    // maintain par with suc
    switch (dir) {
      case Direction::ROOT:
        root = suc;
        break;
      case Direction::LEFT:
        par->left = suc;
        break;
      case Direction::RIGHT:
        par->right = suc;
        break;
      default:
        throw "what the hell have u written!";
        break;
    }
    suc->parent = par;
  }

  void rotateRight(Node* node) {
    if (node == nullptr || node->left == nullptr)
      return;
    Node* par = node->parent;
    Direction dir = node->direction();
    Node* suc = node->left;
    node->left = suc->right;
    suc->right = node;
    maintainRelationship(node);
    maintainRelationship(suc);  // maintain the sons of node and suc

    // maintain par with suc
    switch (dir) {
      case Direction::ROOT:
        root = suc;
        break;
      case Direction::LEFT:
        par->left = suc;
        break;
      case Direction::RIGHT:
        par->right = suc;
        break;
      default:
        throw "what the hell have u written!";
        break;
    }
    suc->parent = par;
  }
  // maintain after insert
  void maintainInsert(Node* ins) {
    if (ins == root) {
      ins->color = BLACK;
      return;
    }
    // case2+3:父节点为根节点
    if (ins->parent->color == BLACK)
      return;
    if (ins->parent == root && root->color == RED) {
      ins->parent->color = BLACK;
      return;
    }
    // case4:N,P,U均为红色，需要重新染色
    if (ins->uncle() != nullptr && ins->uncle()->color == RED) {
      Node *unc = ins->uncle(), *gra = ins->grandparent();
      ins->parent->color = BLACK;
      unc->color = BLACK;
      gra->color = RED;
      maintainInsert(gra);
      root->color = BLACK;
      return;
    }
    // case5:N,P为红色但U是黑色或者U干脆不存在，
    if ((ins->uncle() == nullptr || ins->uncle()->color == BLACK) && ins != root) {
      if (ins->direction() != ins->parent->direction()) {
        Node* par = ins->parent;
        if (ins->direction() == Direction::LEFT)
          rotateRight(ins->parent);
        else
          rotateLeft(ins->parent);
        ins = par;
      }
      if (ins->grandparent() == nullptr)
        return;
      if (ins->parent->direction() == Direction::LEFT)
        rotateRight(ins->grandparent());
      else
        rotateLeft(ins->grandparent());
      ins->parent->color = BLACK;
      ins->sibling()->color = RED;
      root->color = BLACK;
      return;
    }
  }

  pair<Node*, bool> Insert(Node* ins) {
    if (root == nullptr) {
      root = ins;
      ins->parent = nullptr;
      ins->left = nullptr;
      ins->right = nullptr;
      ins->color = BLACK;
      return pair<Node*, bool>(root, true);
    }
    Node *cur = root, *last = nullptr;
    while (cur != nullptr) {
      last = cur;
      if (compare(ins->value.first, cur->value.first))
        cur = cur->left;
      else if (compare(cur->value.first, ins->value.first))
        cur = cur->right;
      else
        // already exists
        return pair<Node*, bool>(cur, false);
    }
    // 此时last就是插入位置
    // 没有考虑last和ins的key相等，因为大概是不存在的
    ins->parent = last;
    if (compare(ins->value.first, last->value.first))
      last->left = ins;
    else
      last->right = ins;
    maintainInsert(ins);
    return pair<Node*, bool>(ins, true);
  }
  // delete
  void maintainDelete(Node* del) {
    if (del == root)
      return;
    Node* sib = del->sibling();
    Direction dir = del->direction();
    // case1 sibling is red,sibling has sons
    if (sib->color == RED) {
      Node* par = del->parent;
      if (dir == Direction::LEFT)
        rotateLeft(par);
      else
        rotateRight(par);
      sib->color = BLACK;
      par->color = RED;
      sib = del->sibling();
    }
    // 两个侄子
    Node* closenep = (dir == Direction::LEFT ? sib->left : sib->right);
    Node* distantnep = (dir == Direction::LEFT ? sib->right : sib->left);
    bool closeblack = closenep == nullptr || closenep->color == BLACK;
    bool distantblack = distantnep == nullptr || distantnep->color == BLACK;
    if (closeblack && distantblack) {
      if (del->parent->color == RED) {
        sib->color = RED;
        del->parent->color = BLACK;
        return;
      } else {
        sib->color = RED;
        maintainDelete(del->parent);
        return;
      }
    } else {
      if (closenep != nullptr && closenep->color == RED) {
        if (dir == Direction::LEFT)
          rotateRight(sib);
        else
          rotateLeft(sib);
        closenep->color = BLACK;
        sib->color = RED;
        sib = del->sibling();
        closenep = (dir == Direction::LEFT ? sib->left : sib->right);
        distantnep = (dir == Direction::LEFT ? sib->right : sib->left);
      }
      // sib=black,c-nep=black,d-nep=red;
      if (dir == Direction::LEFT)
        rotateLeft(del->parent);
      else
        rotateRight(del->parent);
      sib->color = del->parent->color;
      del->parent->color = BLACK;
      if (distantnep != nullptr)
        distantnep->color = BLACK;
      return;
    }
  }

  void Delete(Node* del) {
    Node* todel = del;
    //  case0:only root
    if (siz == 1) {
      delete root;
      root = nullptr;
      return;
    }
    // case1:internal,turn to case 2 3
    if (del->left != nullptr && del->right != nullptr) {
      Node *mini = del->right, *par = del;
      while (mini->left != nullptr)
        par = mini, mini = par->left;
      // swap，分为是父子节点和不是父子节点
      bool isroot = del->parent == nullptr;
      if (del == mini->parent) {
        // par=del，这时可不能随便搞了
        if (!isroot)
          if (del->direction() == Direction::LEFT)
            del->parent->left = mini;
          else
            del->parent->right = mini;
        if (mini->direction() == Direction::LEFT) {
          std::swap(del->right, mini->right);
          del->left = mini->left;
          mini->left = del;
        } else {
          std::swap(del->left, mini->left);
          del->right = mini->right;
          mini->right = del;
        }
        mini->parent = del->parent;
        del->parent = mini;
        maintainRelationship(del), maintainRelationship(mini);
      } else {
        if (!isroot)
          if (del->direction() == Direction::LEFT)
            del->parent->left = mini;
          else
            del->parent->right = mini;
        if (mini->direction() == Direction::LEFT)
          mini->parent->left = del;
        else
          mini->parent->right = del;
        std::swap(del->left, mini->left);
        std::swap(del->right, mini->right);
        std::swap(del->parent, mini->parent);
        maintainRelationship(del), maintainRelationship(mini);
      }
      std::swap(mini->color, del->color);
      //
      if (isroot)
        root = mini;
      todel = del;
    }
    // Case2:Leaf-unlink and remove, but first maintain if black
    if (del->isLeaf()) {
      if (del->color == BLACK)
        maintainDelete(del);  // 双黑，先处理
      // 断链
      if (del->direction() == Direction::LEFT)
        del->parent->left = nullptr;
      else
        del->parent->right = nullptr;
    }
    // Case3:one and only son, son S is red
    else {
      Node *par = del->parent, *rep = (del->left != nullptr ? del->left : del->right);
      // rep为子节点，一定是红色，用其替代del并染黑
      switch (del->direction()) {
        case LEFT:
          par->left = rep;
          break;
        case RIGHT:
          par->right = rep;
          break;
        case ROOT:
          root = rep;
          break;
      }
      if (del != root)
        rep->parent = par;
      // 如果del是红色，rep就是黑色，且不会影响黑高
      if (del->color == BLACK) {
        if (rep->color == RED)
          rep->color = BLACK;
        else
          maintainDelete(rep);
        // 此时有双黑节点出现，需要maintain
      }
    }
    delete todel;
  }
  // search

 public:
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */

  class iterator {
    // iterator 类似指针，++时寻找下一个key值大于它的node（中序遍历）
    // 最坏复杂度可以是log，那么直接按照定义，找右子树的最左点或第一个右路径发生处
   private:
    friend class map;
    friend class const_iterator;
    const map* themap;
    Node* node;
    bool isend;

   public:
    iterator()
        : themap(nullptr) {
      node = nullptr;
      isend = false;
    }
    iterator(const map* _map, Node* _node, bool _isend)
        : themap(_map) {
      node = _node;
      isend = _isend;
    }
    iterator(const iterator& other)
        : themap(other.themap) {
      node = other.node;
      isend = other.isend;
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      if (isend)
        throw invalid_iterator();
      iterator tmp(themap, node, isend);
      if (node->right != nullptr) {
        node = node->right;
        while (node->left != nullptr) {
          node = node->left;
        }
      } else {
        while (node->direction() == Direction::RIGHT)
          node = node->parent;
        if (node->parent == nullptr)  // 最后的节点
          isend = true;               // 转到end()节点
        else
          node = node->parent;
      }
      return tmp;
    }
    /**
     * TODO ++iter
     */
    iterator& operator++() {
      if (isend)
        throw invalid_iterator();
      if (node->right != nullptr) {
        node = node->right;
        while (node->left != nullptr) {
          node = node->left;
        }
      } else {
        while (node->direction() == Direction::RIGHT)
          node = node->parent;
        if (node->parent == nullptr)  // 最后的节点
          isend = true;               // 转到end()节点
        else
          node = node->parent;
      }
      return *this;
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator tmp(themap, node, isend);
      if (isend) {
        if (node == nullptr)
          node = themap->root;
        while (node->parent != nullptr)
          node = node->parent;
        while (node->right != nullptr)
          node = node->right;
        isend = false;
        return tmp;
      }
      if (node->left != nullptr) {
        node = node->left;
        while (node->right != nullptr) {
          node = node->right;
        }
      } else {
        while (node->direction() == Direction::LEFT)
          node = node->parent;
        if (node->parent == nullptr)  // 第一个节点
          throw invalid_iterator();
        else
          node = node->parent;
      }
      return tmp;
    }
    /**
     * TODO --iter
     */
    iterator& operator--() {
      if (isend) {
        if (node == nullptr)
          node = themap->root;
        while (node->parent != nullptr)
          node = node->parent;
        while (node->right != nullptr)
          node = node->right;
        isend = false;
        return *this;
      }
      if (node->left != nullptr) {
        node = node->left;
        while (node->right != nullptr) {
          node = node->right;
        }
      } else {
        while (node->direction() == Direction::LEFT)
          node = node->parent;
        if (node->parent == nullptr)  // 第一个节点
          throw invalid_iterator();
        else
          node = node->parent;
      }
      return *this;
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory).
     */
    value_type& operator*() const {
      if (isend || node == nullptr)
        throw invalid_iterator();
      return node->value;
    }
    bool operator==(const iterator& rhs) const {
      if (themap == rhs.themap) {
        if (isend == rhs.isend) {
          if (isend == true)
            return true;
          else
            return node == rhs.node;
        } else
          return false;
      } else
        return false;
    }
    bool operator==(const const_iterator& rhs) const {
      if (themap == rhs.themap) {
        if (isend == rhs.isend) {
          if (isend == true)
            return true;
          else
            return node == rhs.node;
        } else
          return false;
      } else
        return false;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator& rhs) const {
      return !(*this == rhs);
    }
    bool operator!=(const const_iterator& rhs) const {
      return !(*this == rhs);
    }

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    value_type* operator->() const noexcept {
      if (isend || node == nullptr)
        throw invalid_iterator();
      return &(node->value);
    }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
   private:
    friend class iterator;
    friend class map;
    const map* themap;
    const Node* node;
    bool isend;

   public:
    const_iterator()
        : themap(nullptr), node(nullptr) { isend = false; }
    const_iterator(const map* _map, Node* _node, bool _isend)
        : themap(_map), node(_node) { isend = _isend; }
    const_iterator(const map* _map, const Node* _node, bool _isend)
        : themap(_map), node(_node) { isend = _isend; }
    const_iterator(const const_iterator& other)
        : themap(other.themap), node(other.node) { isend = other.isend; }
    const_iterator(const iterator& other)
        : themap(other.themap), node(other.node) { isend = other.isend; }
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      if (isend)
        throw invalid_iterator();
      const_iterator tmp(themap, node, isend);
      if (node->right != nullptr) {
        node = node->right;
        while (node->left != nullptr) {
          node = node->left;
        }
      } else {
        while (node->direction() == Direction::RIGHT)
          node = node->parent;
        if (node->parent == nullptr)  // 最后的节点
          isend = true;               // 转到end()节点
        else
          node = node->parent;
      }
      return tmp;
    }
    /**
     * TODO ++iter
     */
    const_iterator& operator++() {
      if (isend)
        throw invalid_iterator();
      if (node->right != nullptr) {
        node = node->right;
        while (node->left != nullptr) {
          node = node->left;
        }
      } else {
        while (node->direction() == Direction::RIGHT)
          node = node->parent;
        if (node->parent == nullptr)  // 最后的节点
          isend = true;               // 转到end()节点
        else
          node = node->parent;
      }
      return *this;
    }
    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      const_iterator tmp(themap, node, isend);
      if (isend) {
        if (node == nullptr)
          node = themap->root;
        while (node->parent != nullptr)
          node = node->parent;
        while (node->right != nullptr)
          node = node->right;
        isend = false;
        return tmp;
      }
      if (node->left != nullptr) {
        node = node->left;
        while (node->right != nullptr) {
          node = node->right;
        }
      } else {
        while (node->direction() == Direction::LEFT)
          node = node->parent;
        if (node->parent == nullptr)  // 第一个节点
          throw invalid_iterator();
        else
          node = node->parent;
      }
      return tmp;
    }
    /**
     * TODO --iter
     */
    const_iterator& operator--() {
      if (isend) {
        if (node == nullptr)
          node = themap->root;
        while (node->parent != nullptr)
          node = node->parent;
        while (node->right != nullptr)
          node = node->right;
        isend = false;
        return *this;
      }
      if (node->left != nullptr) {
        node = node->left;
        while (node->right != nullptr) {
          node = node->right;
        }
      } else {
        while (node->direction() == Direction::LEFT)
          node = node->parent;
        if (node->parent == nullptr)  // 第一个节点
          throw invalid_iterator();
        else
          node = node->parent;
      }
      return *this;
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory).
     */
    const value_type& operator*() const {
      if (isend || node == nullptr)
        throw invalid_iterator();
      return node->value;
    }
    bool operator==(const iterator& rhs) const {
      if (themap == rhs.themap) {
        if (isend == rhs.isend) {
          if (isend == true)
            return true;
          else
            return node == rhs.node;
        } else
          return false;
      } else
        return false;
    }
    bool operator==(const const_iterator& rhs) const {
      if (themap == rhs.themap) {
        if (isend == rhs.isend) {
          if (isend == true)
            return true;
          else
            return node == rhs.node;
        } else
          return false;
      } else
        return false;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator& rhs) const {
      return !(*this == rhs);
    }
    bool operator!=(const const_iterator& rhs) const {
      return !(*this == rhs);
    }

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    const value_type* operator->() const noexcept {
      // if (isend || node == nullptr)
      //   throw invalid_iterator();
      return &(node->value);
    }
  };
  /**
   * TODO two constructors
   */
  map() {
    root = nullptr;
    siz = 0;
    top = -1;
    for (int i = 0; i < 20; i++)
      stack[i] = nullptr, ostack[i] = nullptr;
  }
  map(const map& other) {
    if (other.root == nullptr) {
      root = nullptr;
      return;
    }
    root = new Node(other.root);
    siz = other.siz;
    Node *cur = root, *ocur = other.root, *tp, *tpr, *otp, *otpr;
    stack[++top] = cur, ostack[++otop] = ocur;
    while (ocur->left != nullptr) {
      cur->left = new Node(ocur->left);
      cur->left->parent = cur;
      cur = cur->left, ocur = ocur->left;
      stack[++top] = cur, ostack[++otop] = ocur;
    }
    while (top != -1) {
      tp = stack[top], otp = ostack[otop];
      --top, --otop;
      otpr = otp->right;
      if (otpr == nullptr)
        continue;
      tp->right = new Node(otpr);
      tpr = tp->right;
      tpr->parent = tp;
      stack[++top] = tpr, ostack[++otop] = otpr;
      while (otpr->left != nullptr) {
        tpr->left = new Node(otpr->left);
        tpr->left->parent = tpr;
        tpr = tpr->left, otpr = otpr->left;
        stack[++top] = tpr, ostack[++otop] = otpr;
      }
    }
    top = -1, otop = -1;
  }
  /**
   * TODO assignment operator
   */
  map& operator=(const map& other) {
    if (other.root != nullptr && root == other.root)
      return *this;
    clear();
    if (other.root == nullptr) {
      root = nullptr;
      return *this;
    }
    root = new Node(other.root);
    siz = other.siz;
    Node *cur = root, *ocur = other.root, *tp, *tpr, *otp, *otpr;
    stack[++top] = cur, ostack[++otop] = ocur;
    while (ocur->left != nullptr) {
      cur->left = new Node(ocur->left);
      cur->left->parent = cur;
      cur = cur->left, ocur = ocur->left;
      stack[++top] = cur, ostack[++otop] = ocur;
    }
    while (top != -1) {
      tp = stack[top], otp = ostack[otop];
      --top, --otop;
      otpr = otp->right;
      if (otpr == nullptr)
        continue;
      tp->right = new Node(otpr);
      tpr = tp->right;
      tpr->parent = tp;
      stack[++top] = tpr, ostack[++otop] = otpr;
      while (otpr->left != nullptr) {
        tpr->left = new Node(otpr->left);
        tpr->left->parent = tpr;
        tpr = tpr->left, otpr = otpr->left;
        stack[++top] = tpr, ostack[++otop] = otpr;
      }
    }
    top = -1, otop = -1;
    return *this;
  }
  /**
   * TODO Destructors
   */
  ~map() {
    clear();
    root = nullptr;
  }
  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  T& at(const Key& key) {
    iterator res = find(key);
    if (res == end())
      throw index_out_of_bound();
    return res.node->value.second;
  }
  const T& at(const Key& key) const {
    const_iterator res = find(key);
    if (res == cend())
      throw index_out_of_bound();
    return res.node->value.second;
  }
  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  T& operator[](const Key& key) {
    iterator res = find(key);
    if (res == end())
      res = insert(value_type(key, T())).first;
    return res.node->value.second;
  }
  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T& operator[](const Key& key) const {
    const_iterator res = find(key);
    if (res == cend())
      throw index_out_of_bound();
    return res.node->value.second;
  }
  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    if (root == nullptr)
      return end();
    Node* cur = root;
    while (cur->left != nullptr)
      cur = cur->left;
    return iterator(this, cur, false);
  }
  const_iterator cbegin() const {
    if (root == nullptr)
      return cend();
    Node* cur = root;
    while (cur->left != nullptr)
      cur = cur->left;
    return const_iterator(this, cur, false);
  }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() {
    return iterator(this, root, true);
  }
  const_iterator cend() const {
    return const_iterator(this, root, true);
  }
  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const {
    return root == nullptr;
  }
  /**
   * returns the number of elements.
   */
  size_t size() const {
    return siz;
  }
  /**
   * clears the contents
   */
  void clear() {
    if (root == nullptr)
      return;
    top = -1;
    Node* last = nullptr;  // 记录前一个节点
    Node* cur = root;
    while (cur != nullptr) {
      stack[++top] = cur;
      cur = cur->left;
    }
    Node *tp, *tpr;
    while (top != -1) {
      tp = stack[top];
      if (tp->left == nullptr && tp->right == nullptr || last == tp->right || tp->right == nullptr && last == tp->left) {
        delete tp;
        last = tp;
        --top;
      } else {
        tpr = tp->right;
        while (tpr != nullptr) {
          stack[++top] = tpr;
          tpr = tpr->left;
        }
      }
    }
    root = nullptr;
    siz = 0;
  }
  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type& value) {
    // 先插入，插入后对这个节点maintainInsert
    // compare出问题时，应当throw并恢复原状
    Node* ins = new Node(value);
    pair<Node*, bool> res(Insert(ins));
    if (!res.second)
      delete ins;
    else
      ++siz;
    return pair<iterator, bool>(iterator(this, res.first, false), res.second);
  }
  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  void erase(iterator pos) {
    if (pos == end())
      throw invalid_iterator();
    Node *era = pos.node, *rootchecker = era;
    while (rootchecker->parent != nullptr)
      rootchecker = rootchecker->parent;
    if (rootchecker != root)
      throw invalid_iterator();
    // 现在，默认pos的node就是树上节点
    Delete(era);
    --siz;
    return;
  }
  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  iterator find(const Key& key) {
    if (root == nullptr)
      return iterator(this, root, true);
    // 这里存在一个问题，即如果ender没有初始化就会返回空指针
    // 返回就返回吧，先写着先
    Node* cur = root;
    while (cur != nullptr) {
      if (compare(key, cur->value.first))
        cur = cur->left;
      else if (compare(cur->value.first, key))
        cur = cur->right;
      else
        return iterator(this, cur, false);
    }
    return iterator(this, root, true);
  }
  const_iterator find(const Key& key) const {
    if (root == nullptr)
      return const_iterator(this, root, true);
    // 这里存在一个问题，即如果ender没有初始化就会返回空指针
    Node* cur = root;
    while (cur != nullptr) {
      if (compare(key, cur->value.first))
        cur = cur->left;
      else if (compare(cur->value.first, key))
        cur = cur->right;
      else
        return const_iterator(this, cur, false);
    }
    return const_iterator(this, root, true);
  }
  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * 意思就是find，但是返回有没有
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key& key) const {
    if (find(key) == cend())
      return 0;
    else
      return 1;
  }
};

}  // namespace sjtu

#endif
