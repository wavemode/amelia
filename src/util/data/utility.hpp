#pragma once

#include <cstddef>
#include <cstdint>

#include "util/data/runtime_error.hpp"

namespace amelia {

template <typename From, typename To>
concept matches_type = requires(void (*target)(To)) { target; };

template <typename T> inline T &&move(T &arg) noexcept {
  return static_cast<T &&>(arg);
};

template <typename T> inline void swap(T &a, T &b) noexcept {
  T temp(move(a));
  a = move(b);
  b = move(temp);
}

namespace internal {
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

template <typename T, typename Comp>
inline size_t partition(T *slice, size_t start, size_t end, Comp comp) {
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

inline uint64_t set_2_highest_bits_to_01(uint64_t hash) {
  return (hash & 0x3FFFFFFFFFFFFFFFULL) | 0x4000000000000000ULL;
}

inline uint64_t set_2_highest_bits_to_10(uint64_t hash) {
  return (hash & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
}

inline uint64_t set_2_highest_bits_to_00(uint64_t hash) {
  return hash & 0x3FFFFFFFFFFFFFFFULL;
}

inline uint64_t set_2_highest_bits_to_11(uint64_t hash) {
  return hash | 0xC000000000000000ULL;
}

inline uint64_t get_two_highest_bits(uint64_t hash) {
  return hash & 0xC000000000000000ULL;
}

inline bool is_first_bit_set_to_1(uint64_t hash) {
  return (hash & 0x8000000000000000ULL) != 0;
}

inline bool is_second_bit_set_to_1(uint64_t hash) {
  return (hash & 0x4000000000000000ULL) != 0;
}

} // namespace internal

uint64_t hash_str_64(const char *str, size_t len);
uint32_t hash_str_32(const char *str, size_t len);

template <typename T> inline uint64_t bit_hash(const T &value) {
  return hash_str_64(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T> inline uint64_t hash(const T &value) {
  return value.hash_code();
}
inline uint64_t hash(char value) {
  return bit_hash(value);
}
inline uint64_t hash(short value) {
  return bit_hash(value);
}
inline uint64_t hash(int value) {
  return bit_hash(value);
}
inline uint64_t hash(long value) {
  return bit_hash(value);
}
inline uint64_t hash(long long value) {
  return bit_hash(value);
}
inline uint64_t hash(unsigned char value) {
  return bit_hash(value);
}
inline uint64_t hash(unsigned short value) {
  return bit_hash(value);
}
inline uint64_t hash(unsigned int value) {
  return bit_hash(value);
}
inline uint64_t hash(unsigned long value) {
  return bit_hash(value);
}
inline uint64_t hash(unsigned long long value) {
  return bit_hash(value);
}

template <typename T> struct remove_reference {
  using type = T;
};

template <typename T> struct remove_reference<T &> {
  using type = T;
};

template <typename T> struct remove_reference<T &&> {
  using type = T;
};

template <typename T> using remove_reference_t = typename remove_reference<T>::type;

template <typename T> inline constexpr T &&forward(remove_reference_t<T> &param) noexcept {
  return static_cast<T &&>(param);
}

template <typename T> inline constexpr T &&forward(remove_reference_t<T> &&param) noexcept {
  return static_cast<T &&>(param);
}

template <typename T> inline T max(T a, T b) {
  return a > b ? a : b;
}

template <typename T> inline T min(T a, T b) {
  return a < b ? a : b;
}

template <typename T> inline void sort(T *slice, size_t start, size_t end) {
  if (start >= end) {
    return;
  }
  size_t pivot_index = internal::partition(slice, start, end);
  sort(slice, start, pivot_index);
  sort(slice, pivot_index + 1, end);
}

template <typename T, typename Comp>
inline void sort(T *slice, size_t start, size_t end, Comp comp) {
  if (start >= end) {
    return;
  }
  size_t pivot_index = internal::partition(slice, start, end, comp);
  sort(slice, start, pivot_index, comp);
  sort(slice, pivot_index + 1, end, comp);
}

template <typename T> inline void reverse(T *slice, size_t size) {
  for (size_t i = 0; i < size / 2; ++i) {
    swap(slice[i], slice[size - 1 - i]);
  }
}

template <typename T> uintptr_t type_id() {
  static char x;
  return reinterpret_cast<uintptr_t>(&x);
}

struct WithDynamicId {
  template <typename T> bool is() const {
    return type_id<T>() == m_dynamic_id;
  }

  template <typename T> T &as() {
    if (!is<T>()) {
      throw RuntimeError("as() called on a type that is not of the requested type");
    }
    return static_cast<T &>(*this);
  }

  template <typename T> const T &as() const {
    if (!is<T>()) {
      throw RuntimeError("as() called on a type that is not of the requested type");
    }
    return static_cast<const T &>(*this);
  }

protected:
  uintptr_t m_dynamic_id = 0;
};

} // namespace amelia
