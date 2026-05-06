#pragma once

#include <cstddef>
#include <stdexcept>

#include "data/util/abstract_iterator.h"

namespace amelia {

class RuntimeError;
template <typename T> class Slice;
template <typename T> class SliceIterator;
template <typename T> class ConstSlice;
template <typename T> class ConstSliceIterator;

template <typename T> class Slice {
public:
  Slice() noexcept : m_ptr(nullptr), m_len(0) {}

  Slice(T *m_ptr, size_t m_len) noexcept : m_ptr(m_ptr), m_len(m_len) {}

  template <size_t N> explicit Slice(T (&array)[N]) noexcept : m_ptr(array), m_len(N) {}

  explicit Slice(SliceIterator<T> iter) noexcept : m_ptr(iter.ptr()), m_len(iter.size()) {}

  SliceIterator<T> begin() {
    return SliceIterator(*this);
  }
  ConstSliceIterator<T> begin() const {
    return ConstSliceIterator(m_ptr, m_len);
  }

  SliceIterator<T> end() {
    return SliceIterator(m_ptr + m_len, 0);
  }
  ConstSliceIterator<T> end() const {
    return ConstSliceIterator(m_ptr + m_len, 0);
  }

  T *ptr() noexcept {
    return m_ptr;
  }

  size_t size() const noexcept {
    return m_len;
  }

  T &operator[](size_t index) {
    if (index >= m_len) {
      throw RuntimeError("Slice index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const Slice<T> &other) const {
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

  bool operator!=(const Slice<T> &other) const {
    return !(*this == other);
  }

  Slice<T> operator+(size_t offset) {
    if (offset > m_len) {
      throw RuntimeError("Slice offset out of range");
    }
    return Slice(m_ptr + offset, m_len - offset);
  }

  ConstSlice<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice offset out of range");
    }
    return ConstSlice(m_ptr + offset, m_len - offset);
  }

private:
  T *m_ptr;
  size_t m_len;
};

template <typename T> class SliceIterator : public AbstractIterator<T &> {
public:
  SliceIterator() noexcept : m_ptr(nullptr), m_len(0) {}

  SliceIterator(Slice<T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  SliceIterator(T *m_ptr, size_t m_len) noexcept : m_ptr(m_ptr), m_len(m_len) {}

  T *ptr() noexcept {
    return m_ptr;
  }

  size_t size() const noexcept {
    return m_len;
  }

  SliceIterator<T> begin() {
    return *this;
  }
  ConstSliceIterator<T> begin() const {
    return ConstSliceIterator(m_ptr, m_len);
  }

  SliceIterator<T> end() {
    return SliceIterator(m_ptr + m_len, 0);
  }
  ConstSliceIterator<T> end() const {
    return ConstSliceIterator(m_ptr + m_len, 0);
  }

  T &peek() override {
    return **this;
  }

  T &next() override {
    T &value = peek();
    ++(*this);
    return value;
  }

  bool at_end() const noexcept override {
    return m_len == 0;
  }

  T &operator[](size_t index) {
    if (index >= m_len) {
      throw RuntimeError("SliceIterator index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const SliceIterator<T> &other) const noexcept {
    return m_ptr == other.m_ptr;
  }

  bool operator!=(const SliceIterator<T> &other) const noexcept {
    return !(*this == other);
  }

  T &operator*() {
    if (m_len == 0) {
      throw RuntimeError("Dereferencing end of slice");
    }

    return *m_ptr;
  }

  SliceIterator<T> &operator+=(size_t offset) {
    *this = *this + offset;
    return *this;
  }

  SliceIterator<T> &operator++() {
    *this += 1;
    return *this;
  }

  SliceIterator<T> operator++(int) {
    SliceIterator<T> temp = *this;
    ++(*this);
    return temp;
  }

  SliceIterator<T> operator+(size_t offset) {
    if (offset > m_len) {
      throw RuntimeError("Slice iterator offset out of range");
    }
    return SliceIterator(m_ptr + offset, m_len - offset);
  }

  ConstSliceIterator<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice iterator offset out of range");
    }
    return ConstSliceIterator(m_ptr + offset, m_len - offset);
  }

private:
  T *m_ptr;
  size_t m_len;
};

template <typename T> class ConstSlice {
public:
  ConstSlice() noexcept : m_ptr(nullptr), m_len(0) {}

  ConstSlice(const T *m_ptr, size_t m_len) noexcept : m_ptr(m_ptr), m_len(m_len) {}

  ConstSlice(Slice<T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  ConstSlice(const ConstSlice<const T> &slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  template <size_t N> explicit ConstSlice(const T (&array)[N]) noexcept : m_ptr(array), m_len(N) {}

  explicit ConstSlice(ConstSliceIterator<T> iter) noexcept
      : m_ptr(iter.ptr()), m_len(iter.size()) {}

  ConstSliceIterator<T> begin() const {
    return *this;
  }

  ConstSliceIterator<T> end() const {
    return ConstSliceIterator(m_ptr + m_len, 0);
  }

  const T *ptr() const noexcept {
    return m_ptr;
  }

  size_t size() const noexcept {
    return m_len;
  }

  const T &operator[](size_t index) const {
    if (index >= m_len) {
      throw RuntimeError("Slice index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const ConstSlice<T> &other) const {
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

  bool operator!=(const ConstSlice<T> &other) const {
    return !(*this == other);
  }

  ConstSlice<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice offset out of range");
    }
    return ConstSlice(m_ptr + offset, m_len - offset);
  }

  operator Slice<const T>() const {
    return Slice<const T>(m_ptr, m_len);
  }

private:
  const T *m_ptr;
  size_t m_len;
};

template <typename T> class ConstSliceIterator : public AbstractIterator<const T &> {
public:
  ConstSliceIterator() noexcept : m_ptr(nullptr), m_len(0) {}

  ConstSliceIterator(ConstSlice<T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  ConstSliceIterator(Slice<T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  ConstSliceIterator(Slice<const T> slice) noexcept : m_ptr(slice.ptr()), m_len(slice.size()) {}

  ConstSliceIterator(const T *m_ptr, size_t m_len) noexcept : m_ptr(m_ptr), m_len(m_len) {}

  const T *ptr() const noexcept {
    return m_ptr;
  }

  size_t size() const noexcept {
    return m_len;
  }

  ConstSliceIterator<T> begin() const {
    return ConstSliceIterator(m_ptr, m_len);
  }

  ConstSliceIterator<T> end() const {
    return ConstSliceIterator(m_ptr + m_len, 0);
  }

  const T &operator[](size_t index) {
    if (index >= m_len) {
      throw RuntimeError("SliceIterator index out of range");
    }
    return m_ptr[index];
  }

  bool operator==(const ConstSliceIterator<T> &other) const {
    return m_ptr == other.m_ptr;
  }

  bool operator!=(const ConstSliceIterator<T> &other) const {
    return !(*this == other);
  }

  const T &operator*() {
    if (m_len == 0) {
      throw RuntimeError("Dereferencing end of slice");
    }

    return *m_ptr;
  }

  const T &peek() override {
    return **this;
  }

  const T &next() override {
    const T &value = peek();
    ++(*this);
    return value;
  }

  bool at_end() const noexcept override {
    return m_len == 0;
  }

  ConstSliceIterator<T> &operator+=(size_t offset) {
    *this = *this + offset;
    return *this;
  }

  ConstSliceIterator<T> &operator++() {
    *this += 1;
    return *this;
  }

  ConstSliceIterator<T> operator++(int) {
    ConstSliceIterator<T> temp = *this;
    ++(*this);
    return temp;
  }

  ConstSliceIterator<T> operator+(size_t offset) const {
    if (offset > m_len) {
      throw RuntimeError("Slice iterator offset out of range");
    }
    return ConstSliceIterator(m_ptr + offset, m_len - offset);
  }

private:
  const T *m_ptr;
  size_t m_len;
};

} // namespace amelia
