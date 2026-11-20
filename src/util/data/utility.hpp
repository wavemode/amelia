#pragma once

#include <cstddef>
#include <cstdint>

namespace amelia {

[[noreturn]] void throw_runtime_error(const char *message);

template <bool B, typename T = void> struct enable_if {};

template <typename T> struct enable_if<true, T> {
  using type = T;
};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename T, T v> struct integral_constant {
  static constexpr T value = v;
  using value_type = T;
  using type = integral_constant;
  constexpr operator value_type() const noexcept { return value; }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template <typename> constexpr bool always_false_v = false;

template <typename T> T &&declval() noexcept {
  static_assert(always_false_v<T>,
                "declval must not be used in an evaluated context");
}

namespace internal {

template <typename To> void test_aux(To) noexcept;

template <typename From, typename To,
          typename = decltype(test_aux<To>(amelia::declval<From>()))>
true_type test_convertible(int);

template <typename, typename> false_type test_convertible(...);

template <typename T> struct is_void : false_type {};
template <> struct is_void<void> : true_type {};
template <> struct is_void<const void> : true_type {};
template <> struct is_void<volatile void> : true_type {};
template <> struct is_void<const volatile void> : true_type {};

} // namespace internal

template <typename From, typename To>
struct is_convertible
    : amelia::integral_constant<
          bool,
          (decltype(amelia::internal::test_convertible<From, To>(0))::value) ||
              (amelia::internal::is_void<From>::value &&
               amelia::internal::is_void<To>::value)> {};

template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

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
template <typename T>
inline size_t partition(T *slice, size_t start, size_t end) {
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

constexpr size_t HASH_STATE_SIZE = 303;
constexpr size_t HASH_BLOCK_SIZE = 512;

} // namespace internal

uint64_t hash_str_64(const char *str, size_t len);
uint32_t hash_str_32(const char *str, size_t len);

class HashState {
  alignas(alignof(max_align_t)) char m_data[internal::HASH_BLOCK_SIZE];
  alignas(alignof(max_align_t)) char m_state[internal::HASH_STATE_SIZE];
  bool m_initialized;
  size_t m_length;

public:
  HashState() noexcept;
  void init() noexcept;
  void update(const char *data, size_t len);
  uint64_t finalize();
};

template <typename T> inline uint64_t bit_hash(const T &value) {
  return hash_str_64(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T> inline uint64_t hash(const T &value) {
  return value.hash_code();
}
inline uint64_t hash(char value) { return bit_hash(value); }
inline uint64_t hash(short value) { return bit_hash(value); }
inline uint64_t hash(int value) { return bit_hash(value); }
inline uint64_t hash(long value) { return bit_hash(value); }
inline uint64_t hash(long long value) { return bit_hash(value); }
inline uint64_t hash(unsigned char value) { return bit_hash(value); }
inline uint64_t hash(unsigned short value) { return bit_hash(value); }
inline uint64_t hash(unsigned int value) { return bit_hash(value); }
inline uint64_t hash(unsigned long value) { return bit_hash(value); }
inline uint64_t hash(unsigned long long value) { return bit_hash(value); }

template <typename T> struct remove_reference {
  using type = T;
};

template <typename T> struct remove_reference<T &> {
  using type = T;
};

template <typename T> struct remove_reference<T &&> {
  using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

template <typename T>
inline constexpr T &&forward(remove_reference_t<T> &param) noexcept {
  return static_cast<T &&>(param);
}

template <typename T>
inline constexpr T &&forward(remove_reference_t<T> &&param) noexcept {
  return static_cast<T &&>(param);
}

template <typename T> inline T max(T a, T b) { return a > b ? a : b; }

template <typename T> inline T min(T a, T b) { return a < b ? a : b; }

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

struct Dynamic {
  template <typename T> bool is() const {
    return dynamic_cast<const T *>(this) != nullptr;
  }

  template <typename T> T &as() {
    auto *ptr = dynamic_cast<T *>(this);
    if (!ptr) {
      throw_runtime_error(
          "as() called on a type that is not of the requested type");
    }
    return *ptr;
  }

  template <typename T> const T &as() const {
    const auto *ptr = dynamic_cast<const T *>(this);
    if (!ptr) {
      throw_runtime_error(
          "as() called on a type that is not of the requested type");
    }
    return *ptr;
  }

  virtual ~Dynamic() = default;
};

} // namespace amelia
