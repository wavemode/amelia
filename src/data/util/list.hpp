#pragma once

#include <cstddef>
#include <cstdlib>

#include "data/util/abstract_list.hpp"
#include "data/util/runtime_error.hpp"
#include "data/util/utility.hpp"

namespace amelia {

template <typename T> class Slice;
template <typename T> class SliceIterator;
template <typename T> class ConstSlice;
template <typename T> class ConstSliceIterator;

template <typename T> class List : public AbstractList<T> {
public:
  using value_type = T;

  List() noexcept = default;

  explicit List(ConstSlice<T> slice) noexcept {
    append(slice);
  }

  template <size_t N> explicit List(const T (&array)[N]) noexcept : List(ConstSlice(array, N)) {}

  List(const List<T> &other) noexcept : List(other.data()) {}

  List(List<T> &&other) noexcept
      : m_buffer(other.m_buffer), m_size(other.m_size), m_capacity(other.m_capacity) {
    other.m_buffer = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  ~List() noexcept {
    clear();
    std::free(m_buffer);
  }

  SliceIterator<T> begin() noexcept {
    return SliceIterator(m_buffer, m_size);
  }
  ConstSliceIterator<T> begin() const noexcept {
    return ConstSliceIterator(m_buffer, m_size);
  }

  SliceIterator<T> end() noexcept {
    return SliceIterator(m_buffer + m_size, 0);
  }
  ConstSliceIterator<T> end() const noexcept {
    return ConstSliceIterator(m_buffer + m_size, 0);
  }

  Slice<T> data() noexcept {
    return Slice(m_buffer, m_size);
  }
  ConstSlice<T> data() const noexcept {
    return ConstSlice(m_buffer, m_size);
  }

  size_t size() const noexcept override {
    return m_size;
  }

  void push_back(T value) override {
    ensure_capacity(m_size + 1);
    initialize(m_size, move(value));
    ++m_size;
  }

  void pop_back() {
    if (m_size == 0) {
      throw RuntimeError("Cannot pop from empty list");
    }
    --m_size;
    m_buffer[m_size].~T();
  }

  void reserve(size_t new_capacity) {
    ensure_capacity(new_capacity);
  }

  void clear() noexcept {
    while (m_size > 0) {
      pop_back();
    }
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    ensure_capacity(m_size + 1);
    initialize(m_size, T(amelia::forward<Args>(args)...));
    return m_buffer[m_size++];
  }

  void append(ConstSlice<T> slice) override {
    *this += slice;
  }

  void assign(ConstSlice<T> slice) override {
    clear();
    *this += slice;
  }

  bool has(const T &value) const {
    for (const T &item : *this) {
      if (item == value) {
        return true;
      }
    }
    return false;
  }

  void reverse() noexcept {
    amelia::reverse(m_buffer, m_size);
  }

  void sort() {
    amelia::sort(m_buffer, 0, m_size);
  }

  template <typename Compare> void sort(Compare comp) {
    amelia::sort(m_buffer, 0, m_size, comp);
  }

  List &operator=(const List<T> &other) noexcept {
    if (this != &other) {
      assign(other.data());
    }
    return *this;
  }

  List &operator=(List<T> &&other) noexcept {
    if (this != &other) {
      clear();
      std::free(m_buffer);
      m_buffer = other.m_buffer;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_buffer = nullptr;
      other.m_size = 0;
      other.m_capacity = 0;
    }
    return *this;
  }

  T &operator[](size_t index) override {
    if (index >= size()) {
      throw RuntimeError("List index out of range");
    }
    return m_buffer[index];
  }

  const T &operator[](size_t index) const {
    if (index >= size()) {
      throw RuntimeError("List index out of range");
    }
    return m_buffer[index];
  }

  List<T> &operator+=(const List<T> &other) {
    ensure_capacity(m_size + other.m_size);
    for (size_t i = 0; i < other.m_size; ++i) {
      initialize(m_size + i, other.m_buffer[i]);
    }
    m_size += other.m_size;
    return *this;
  }

  List<T> &operator+=(ConstSlice<T> slice) {
    ensure_capacity(m_size + slice.size());
    for (size_t i = 0; i < slice.size(); ++i) {
      initialize(m_size + i, slice.ptr()[i]);
    }
    m_size += slice.size();
    return *this;
  }

  List<T> operator+(const List<T> &other) const {
    List<T> result = *this;
    result += other.data();
    return result;
  }

  List<T> operator+(ConstSlice<T> slice) const {
    List<T> result = *this;
    result += slice;
    return result;
  }

  bool operator==(const List<T> &other) const {
    if (size() != other.size()) {
      return false;
    }
    for (size_t i = 0; i < size(); ++i) {
      if (m_buffer[i] != other.m_buffer[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator==(ConstSlice<T> slice) const {
    if (size() != slice.size()) {
      return false;
    }
    for (size_t i = 0; i < size(); ++i) {
      if (m_buffer[i] != slice[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const List<T> &other) const {
    return !(*this == other);
  }
  bool operator!=(ConstSlice<T> other) const {
    return !(*this == other);
  }

private:
  void initialize(size_t index, const T &value) noexcept {
    new (&m_buffer[index]) T(value);
  }

  void initialize(size_t index, T &&value) noexcept {
    new (&m_buffer[index]) T(move(value));
  }

  void ensure_capacity(size_t new_capacity) noexcept {
    if (new_capacity > m_capacity) {
      size_t new_cap = amelia::max(new_capacity, m_capacity == 0 ? 8 : (m_capacity * 2));
      T *old_buffer = m_buffer;
      m_buffer = static_cast<T *>(std::malloc(new_cap * sizeof(T)));
      if (m_buffer == nullptr) {
        std::abort();
      }
      for (size_t i = 0; i < m_size; ++i) {
        initialize(i, move(old_buffer[i]));
      }
      std::free(old_buffer);
      m_capacity = new_cap;
    }
  }

  T *m_buffer = nullptr;
  size_t m_size = 0;
  size_t m_capacity = 0;
};

} // namespace amelia
