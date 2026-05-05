#pragma once

#include <cstddef>
#include <stdexcept>

#include "data/util/abstract_iterator.h"

namespace amelia {

class RuntimeError;
template <typename T> class SliceIterator;

template <typename T> class Slice {
public:
  Slice() noexcept : m_ptr(nullptr), m_len(0) {}

  Slice(T *m_ptr, size_t m_len) noexcept : m_ptr(m_ptr), m_len(m_len) {}

  template <size_t N> explicit Slice(T (&array)[N]) noexcept : m_ptr(array), m_len(N) {}

  explicit Slice(SliceIterator<T> iter) noexcept : m_ptr(iter.ptr()), m_len(iter.size()) {}

  SliceIterator<T> begin() const { return SliceIterator(*this); }

  SliceIterator<T> end() const { return SliceIterator(Slice(m_ptr + m_len, 0)); }

  T *ptr() const noexcept { return m_ptr; }

  size_t size() const noexcept { return m_len; }

  T &operator[](size_t index) const {
    if (index >= m_len) {
      throw RuntimeError("Slice index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const Slice<T> &other) const noexcept {
    if (m_len != other.m_len) {
      return false;
    }
    if (m_ptr == other.m_ptr) {
      return true;
    }
    for (size_t i = 0; i < m_len; ++i) {
      if (m_ptr[i] != other.m_ptr[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Slice<T> &other) const noexcept { return !(*this == other); }

  Slice<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice offset out of range");
    }
    return Slice(m_ptr + offset, m_len - offset);
  }

private:
  T *m_ptr;
  size_t m_len;
};

template <typename T> class SliceIterator : public AbstractIterator<T &> {
public:
  SliceIterator() noexcept : m_ptr(nullptr), m_len(0) {}

  SliceIterator(Slice<T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  T *ptr() const noexcept { return m_ptr; }

  size_t size() const noexcept { return m_len; }

  SliceIterator<T> begin() const { return *this; }
  SliceIterator<T> end() const { return Slice(m_ptr + m_len, 0); }

  T &operator[](size_t index) const {
    if (index >= m_len) {
      throw RuntimeError("SliceIterator index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const SliceIterator<T> &other) const noexcept { return m_ptr == other.m_ptr; }

  bool operator!=(const SliceIterator<T> &other) const noexcept { return !(*this == other); }

  T &operator*() const {
    if (m_len == 0) {
      throw RuntimeError("Dereferencing end of slice");
    }

    return *m_ptr;
  }

  T &peek() override { return **this; }

  SliceIterator<T> &operator+=(size_t offset) {
    *this = *this + offset;
    return *this;
  }

  SliceIterator<T> &operator++() {
    *this += 1;
    return *this;
  }

  SliceIterator<T> operator++(int) {
    Slice<T> temp = *this;
    ++(*this);
    return temp;
  }

  T &next() override {
    T &value = peek();
    ++(*this);
    return value;
  }

  bool at_end() const override { return m_len == 0; }

  SliceIterator<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice iterator offset out of range");
    }
    return Slice(m_ptr + offset, m_len - offset);
  }

private:
  T *m_ptr;
  size_t m_len;
};

} // namespace amelia
