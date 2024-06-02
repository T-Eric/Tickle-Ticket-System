#ifndef SJTU_UTILITY_HPP
#define SJTU_UTILITY_HPP

#include <utility>

namespace sjtu {

template <class T1, class T2>
class pair {
 public:
  T1 first;   // first    element
  T2 second;  // second   element

  pair() = default;
  pair(const pair& other)
      : first(other.first), second(other.second) {}
  pair(pair&& other)
      : first(std::move(other.first)), second(std::move(other.second)) {}

  pair& operator=(const pair& other) {
    if (this != &other) {
      first = other.first;
      second = other.second;
    }
    return *this;
  }

  template <class U1 = T1, class U2 = T2>
  pair(U1&& x, U2&& y)
      : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

  template <class U1, class U2>
  pair(const pair<U1, U2>& other)
      : first(other.first), second(other.second) {}

  template <class U1, class U2>
  pair(pair<U1, U2>&& other)
      : first(std::move(other.first)), second(std::move(other.second)) {}
};

template <class T1, class T2>
pair(T1, T2) -> pair<T1, T2>;

}  // namespace sjtu

#endif