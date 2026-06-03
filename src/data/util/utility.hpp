#pragma once

#include <cstdint>
#include <cstddef>

namespace amelia {

uint64_t hash_str(const char *str, size_t len);

template <typename T>
struct remove_reference { using type = T; };

template <typename T>
struct remove_reference<T&> { using type = T; };

template <typename T>
struct remove_reference<T&&> { using type = T; };

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

template <typename T>
inline constexpr T&& forward(remove_reference_t<T>& param) noexcept {
    return static_cast<T&&>(param);
}

template <typename T>
inline constexpr T&& forward(remove_reference_t<T>&& param) noexcept {
    return static_cast<T&&>(param);
}

template <typename T>
inline T max(T a, T b) {
  return a > b ? a : b;
}

template <typename T>
inline T && move(T &arg) noexcept {
  return static_cast<T &&>(arg);
};

template <typename T> inline void swap(T &a, T &b) noexcept {
  T temp = move(a);
  a = move(b);
  b = move(temp);
}

namespace util_internal {
template <typename T> inline size_t partition(T *slice, size_t start, size_t end) {
  T &pivot = slice[end - 1];
  size_t i = start;
  for (size_t j = start; j < end - 1; ++j) {
    if (slice[j] < pivot) {
      swap(slice[i], slice[j]);
      ++i;
    }
  }
  swap(slice[i], slice[end - 1]);
  return i;
}
} // namespace util_internal

template <typename T> inline void sort(T *slice, size_t start, size_t end) {
  if (start >= end) {
    return;
  }
  size_t pivot_index = util_internal::partition(slice, start, end);
  sort(slice, start, pivot_index);
  sort(slice, pivot_index + 1, end);
}

namespace util_internal {
template <typename T, typename Comp> inline size_t partition(T *slice, size_t start, size_t end, Comp comp) {
  T &pivot = slice[end - 1];
  size_t i = start;
  for (size_t j = start; j < end - 1; ++j) {
    if (comp(slice[j], pivot)) {
      swap(slice[i], slice[j]);
      ++i;
    }
  }
  swap(slice[i], slice[end - 1]);
  return i;
}
} // namespace util_internal

template <typename T, typename Comp> inline void sort(T *slice, size_t start, size_t end, Comp comp) {
  if (start >= end) {
    return;
  }
  size_t pivot_index = util_internal::partition(slice, start, end, comp);
  sort(slice, start, pivot_index, comp);
  sort(slice, pivot_index + 1, end, comp);
}

template <typename T> inline void reverse(T *slice, size_t size) {
  for (size_t i = 0; i < size / 2; ++i) {
    swap(slice[i], slice[size - 1 - i]);
  }
}

} // namespace amelia
