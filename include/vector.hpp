#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <cstring>
#include <iostream>

namespace sjtu {
template <typename T>
class Allocator {
 public:
  T* allocate(size_t siz) {
    return (T*)malloc(sizeof(T) * siz);
  }  // 分配内存
  void deallocate(T* p) {  // void *p?
    free(p);
  }  // 就是delete[]p
  void construct(T* p, const T& value) {
    new (p) T(value);
  }  // 就是p=new T(value)
  void destroy(T* p) {
    p->~T();
  }  // 就是手动析构
};
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T, typename Alloc = Allocator<T>>
class vector {
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
    // Type traits is a C++ feature for describing certain properties of a type.
    // For instance, for an iterator, iterator::value_type is the type that the
    // iterator points to.
    // STL algorithms and containers may use these type_traits (e.g. the following
    // typedef) to work properly. In particular, without the following code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
    // About iterator_category: https://en.cppreference.com/w/cpp/iterator
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::output_iterator_tag;

   private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    vector<T>* _head;
    int _index;

   public:
    friend class const_iterator;
    friend class vector;

    iterator(vector<T>* __head, int __index) {
      _head = __head;
      _index = __index;
    }
    ~iterator() {
      _head = nullptr;
    }
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    iterator operator+(const int& n) const {
      return iterator(_head, _index + n);
    }
    iterator operator-(const int& n) const {
      if (_index <= n)
        throw index_out_of_bound();
      return iterator(_head, _index - n);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw invaild_iterator.
    int operator-(const iterator& rhs) const {
      if (_head != rhs._head)
        throw invalid_iterator();
      return _index - rhs._index;
    }
    iterator& operator+=(const int& n) {
      _index += n;
      return *this;
    }
    iterator& operator-=(const int& n) {
      if (_index < n)
        throw invalid_iterator();
      _index -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      iterator tmp = *this;
      ++_index;
      return tmp;
    }
    /**
     * TODO ++iter
     */
    iterator& operator++() {
      ++_index;
      return *this;
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator tmp = *this;
      --_index;
      return tmp;
    }
    /**
     * TODO --iter
     */
    iterator& operator--() {
      --_index;
      return *this;
    }
    /**
     * TODO *it
     */
    T& operator*() const {
      return _head->operator[](_index);
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    bool operator==(const iterator& rhs) const {
      return _head == rhs._head && _index == rhs._index;
    }
    bool operator==(const const_iterator& rhs) const {
      return _head == rhs._head && _index == rhs._index;
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
  };
  /**
   * TODO
   * has same function as iterator, just for a const object.
   */
  class const_iterator {
   public:
    friend class iterator;
    friend class vector;

    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::output_iterator_tag;
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    const_iterator(const vector<T>* __head, int __index) {
      _head = __head;
      _index = __index;
    }
    ~const_iterator() {
      _head = nullptr;
    }
    /**
     * return a new const_iterator which pointer n-next elements
     * as well as operator-
     */
    const_iterator operator+(const int& n) const {
      return const_iterator(_head, _index + n);
    }
    const_iterator operator-(const int& n) const {
      if (_index <= n)
        throw index_out_of_bound();
      return const_iterator(_head, _index - n);
    }
    // return the distance between two const_iterators,
    // if these two const_iterators point to different vectors, throw invaild_const_iterator.
    int operator-(const const_iterator& rhs) const {
      if (_head != rhs._head)
        throw invalid_iterator();
      return _index - rhs._index;
    }
    const_iterator& operator+=(const int& n) {
      _index += n;
      return *this;
    }
    const_iterator& operator-=(const int& n) {
      if (_index < n)
        throw invalid_iterator();
      _index -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++_index;
      return tmp;
    }
    /**
     * TODO ++iter
     */
    const_iterator& operator++() {
      ++_index;
      return *this;
    }
    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      --_index;
      return tmp;
    }
    /**
     * TODO --iter
     */
    const_iterator& operator--() {
      --_index;
      return *this;
    }
    /**
     * TODO *it
     */
    const T& operator*() const {
      return _head->operator[](_index);
    }
    bool operator==(const iterator& rhs) const {
      return _head == rhs._head && _index == rhs._index;
    }
    bool operator==(const const_iterator& rhs) const {
      return _head == rhs._head && _index == rhs._index;
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

   private:
    const vector<T>* _head;
    int _index;
  };

 private:
  T* _first;  // 开头
  T* _last;   // 末尾元素的后一位
  T* _end;    // 分配的空间允许的最大位置
  Alloc _alloc;

  bool full() {
    return _end == _last;
  }
  bool lessen() {
    return (_last - _first) * 3 < _end - _first;
  }
  void expand() {  // 扩容，默认两倍
    int siz = _end - _first;
    T* tmp = _alloc.allocate(siz * 2);
    for (int i = 0; i < siz; ++i)
      _alloc.construct(tmp + i, _first[i]);
    for (T* p = _first; p != _last; ++p)
      _alloc.destroy(p);
    _alloc.deallocate(_first);
    _first = tmp;
    _last = _first + siz;
    _end = _first + siz * 2;
  }
  void shrink() {  // 缩水，默认二分之一
    int siz = _end - _first, sum = _last - _first;
    T* tmp = _alloc.allocate(siz / 2);
    for (int i = 0; i < sum; ++i)
      _alloc.construct(tmp + i, _first[i]);
    for (T* p = _first; p != _last; ++p)
      _alloc.destroy(p);
    _alloc.deallocate(_first);
    _first = tmp;
    _last = _first + sum;
    _end = _first + siz / 2;
  }

  /**
   * TODO Constructs
   * At least two: default constructor, copy constructor
   */
 public:
  vector(int siz = 16) {
    _first = _alloc.allocate(siz);
    _last = _first;
    _end = _first + siz;
  }
  vector(const vector& other) {
    int siz = other._end - other._first;
    _first = _alloc.allocate(siz);
    int len = other._last - other._first;
    for (int i = 0; i < len; ++i)
      _alloc.construct(_first + i, other._first[i]);
    _last = _first + len;
    _end = _first + siz;
  }
  /**
   * TODO Destructor
   */
  ~vector() {
    for (T* p = _first; p != _last; ++p)
      _alloc.destroy(p);
    _alloc.deallocate(_first);
    _first = _last = _end = nullptr;
  }
  /**
   * TODO Assignment operator
   */
  vector& operator=(const vector& other) {
    if (this != &other) {
      for (T* p = _first; p != _last; ++p)
        _alloc.destroy(p);
      _alloc.deallocate(_first);
      int siz = _last - _first;
      _first = _alloc.allocate(siz);
      int len = other._last - other._first;
      for (int i = 0; i < len; ++i)
        _alloc.construct(_first + i, other._first[i]);
      _last = _first + len;
      _end = _first + siz;
    }
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  T& at(const size_t& pos) {
    if (pos > size())
      throw index_out_of_bound();
    return _first[pos];
  }
  const T& at(const size_t& pos) const {
    if (pos > size())
      throw index_out_of_bound();
    return _first[pos];
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
  T& operator[](const size_t& pos) {
    if (pos > size())
      throw index_out_of_bound();
    return *(_first + pos);
  }
  const T& operator[](const size_t& pos) const {
    if (pos > size())
      throw index_out_of_bound();
    return *(_first + pos);
  }
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  const T& front() const {
    if (size() == 0)
      throw container_is_empty();
    return *_first;
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  const T& back() const {
    if (size() == 0)
      throw container_is_empty();
    return *(_last - 1);
  }
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() {
    return iterator(this, 0);
  }
  const_iterator cbegin() const {
    return const_iterator(this, 0);
  }
  /**
   * returns an iterator to the end.
   */
  iterator end() {
    return iterator(this, size());
  }
  const_iterator cend() const {
    return const_iterator(this, size());
  }
  /**
   * checks whether the container is empty
   */
  bool empty() const {
    return _first == _last;
  }
  /**
   * returns the number of elements
   */
  size_t size() const {
    return _last - _first;
  }
  /**
   * clears the contents
   */
  void clear() {
    for (T* p = _first; p != _last; ++p)
      _alloc.destroy(p);
    _alloc.deallocate(_first);
    int siz = 16;
    _first = _alloc.allocate(siz);
    _last = _first;
    _end = _first + siz;
  }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  iterator insert(iterator pos, const T& value) {
    if (full())
      expand();
    _alloc.construct(_last, value);
    ++_last;
    // 先在last处加一个，后面再进行覆盖
    for (T* cur = _last - 1; cur != _first + pos._index; --cur)
      *cur = *(cur - 1);
    *(_first + pos._index) = value;
    return pos;
  }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
   */
  iterator insert(const size_t& ind, const T& value) {
    if (ind < 0 || ind > size())
      throw index_out_of_bound();
    if (full())
      expand();
    _alloc.construct(_last, value);
    ++_last;
    T* cur;
    for (T* cur = _last; cur != (_first + ind); --cur)
      *cur = *(cur - 1);
    *(_first + ind) = value;
    return iterator(this, ind);
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is returned.
   */
  iterator erase(iterator pos) {
    if (lessen() && _end - _first > 16)
      shrink();  // 至少经历一次expand才能shrink
    for (T* cur = _first + pos._index; cur != _last - 1; ++cur)
      *cur = *(cur + 1);
    _alloc.destroy(_last - 1);
    --_last;
    return pos;
  }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  iterator erase(const size_t& ind) {
    if (ind < 0 || ind >= size())
      throw index_out_of_bound();
    if (lessen() && _end - _first > 16)
      shrink();  // 至少经历一次expand才能shrink
    for (T* cur = _first + ind; cur != _last - 1; ++cur)
      *cur = *(cur + 1);
    _alloc.destroy(_last - 1);
    --_last;
    return iterator(this, ind);
  }
  /**
   * adds an element to the end.
   */
  void push_back(const T& value) {
    if (full())
      expand();
    _alloc.construct(_last, value);
    ++_last;
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  void pop_back() {
    if (size() == 0)
      throw container_is_empty();
    if (lessen() && _end - _first > 16)
      shrink();  // 至少经历一次expand才能shrink
    --_last;
    _alloc.destroy(_last);
  }
};

}  // namespace sjtu

#endif
