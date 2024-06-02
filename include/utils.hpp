#ifndef SJTU_TICKET_UTILS_HPP
#define SJTU_TICKET_UTILS_HPP

#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "map.hpp"
#include "utility.hpp"
#include "vector.hpp"

using std::cin;
using std::cout;
using std::string;

namespace sjtu {

// 48长度string
struct String {
  char str[48];

  String() {
    memset(str, 0, sizeof(str));
  }
  String(const char* s) {
    strcpy(str, s);
  }
  String(const String& s) {
    strcpy(str, s.str);
  }
  ~String() = default;
  String& operator=(const String& s) {
    if (this != &s)
      strcpy(str, s.str);
    return *this;
  }
  String& operator=(const char* s) {
    strcpy(str, s);
    return *this;
  }
  String& operator=(const string& s) {
    strcpy(str, s.c_str());
    return *this;
  }
  string toString() {
    return string(str);
  }
  friend bool operator==(const String& lhs, const String& rhs) {
    return !strcmp(lhs.str, rhs.str);
  }
  friend bool operator<(const String& lhs, const String& rhs) {
    return strcmp(lhs.str, rhs.str) < 0;
  }
  friend bool operator<=(const String& lhs, const String& rhs) {
    return lhs < rhs || lhs == rhs;
  }
  friend bool operator!=(const String& lhs, const String& rhs) {
    return !(lhs == rhs);
  }
  friend std::ostream& operator<<(std::ostream& os, const String& s) {
    os << s.str;
    return os;
  }
};

// 32长度string
struct Word {
  char str[32];

  Word() {
    memset(str, 0, sizeof(str));
  }
  Word(const char* s) {
    strcpy(str, s);
  }
  Word(const Word& s) {
    strcpy(str, s.str);
  }
  ~Word() = default;
  Word& operator=(const Word& s) {
    if (this != &s)
      strcpy(str, s.str);
    return *this;
  }
  Word& operator=(const char* s) {
    strcpy(str, s);
    return *this;
  }
  Word& operator=(const string& s) {
    strcpy(str, s.c_str());
    return *this;
  }
  string toString() {
    return string(str);
  }
  friend bool operator==(const Word& lhs, const Word& rhs) {
    return !strcmp(lhs.str, rhs.str);
  }
  friend bool operator<(const Word& lhs, const Word& rhs) {
    return strcmp(lhs.str, rhs.str) < 0;
  }
  friend bool operator<=(const Word& lhs, const Word& rhs) {
    return lhs < rhs || lhs == rhs;
  }
  friend bool operator!=(const Word& lhs, const Word& rhs) {
    return !(lhs == rhs);
  }
  friend std::ostream& operator<<(std::ostream& os, const Word& s) {
    os << s.str;
    return os;
  }
};

// 24长度String，因为只有username和trainID会用，就叫ID了
struct ID {
  char str[24];

  ID() {
    memset(str, 0, sizeof(str));
  }
  ID(const char* s) {
    strcpy(str, s);
  }
  ID(const ID& s) {
    strcpy(str, s.str);
  }
  ~ID() = default;
  ID& operator=(const ID& s) {
    if (this != &s)
      strcpy(str, s.str);
    return *this;
  }
  ID& operator=(const char* s) {
    strcpy(str, s);
    return *this;
  }
  ID& operator=(const string& s) {
    strcpy(str, s.c_str());
    return *this;
  }
  string toString() {
    return string(str);
  }
  friend bool operator==(const ID& lhs, const ID& rhs) {
    return !strcmp(lhs.str, rhs.str);
  }
  friend bool operator<(const ID& lhs, const ID& rhs) {
    return strcmp(lhs.str, rhs.str) < 0;
  }
  friend bool operator<=(const ID& lhs, const ID& rhs) {
    return lhs < rhs || lhs == rhs;
  }
  friend bool operator!=(const ID& lhs, const ID& rhs) {
    return !(lhs == rhs);
  }
  friend std::ostream& operator<<(std::ostream& os, const ID& s) {
    os << s.str;
    return os;
  }
};

// 字符串分割函数
void SplitString(vector<std::string>& v, const string& s, char delim = '|') {
  v.clear();
  string token;
  size_t start = 0;
  size_t end = s.find(delim);

  while (end != std::string::npos) {
    token = s.substr(start, end - start);
    v.push_back(token);
    start = end + 1;
    end = s.find(delim, start);
  }

  v.push_back(s.substr(start, end));
}

template <class T>
void MergeSort(vector<T>& v, int l, int r, T* a, bool (*comp)(const T&, const T&)) {
  if (l == r)
    return;
  int mid = (l + r) >> 1;
  MergeSort(v, l, mid, a, comp);
  MergeSort(v, mid + 1, r, a, comp);
  int i = l, j = mid + 1, k = l;
  while (i <= mid && j <= r) {
    if (comp(v[i], v[j]))
      a[k++] = a[i++];
    else
      a[k++] = a[j++];
  }
  while (i <= mid)
    a[k++] = v[i++];
  while (j <= r)
    a[k++] = v[j++];
  for (int i = l; i <= r; i++)
    v[i] = a[i];
}
template <class T>
void Sort(vector<T>& v, bool (*comp)(const T&, const T&) = std::less<T>()) {
  T* a = new T[v.size()];
  for (int i = 0; i < v.size(); i++)
    a[i] = v[i];
  MergeSort(v, 0, v.size() - 1, a, comp);
  delete[] a;
}

}  // namespace sjtu

#endif  // !SJTU_TICKETS_UTILS_HPP