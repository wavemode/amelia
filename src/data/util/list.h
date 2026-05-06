#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <vector>

#include "data/util/abstract_list.h"

namespace amelia {

class RuntimeError;
template <typename T> class Slice;
template <typename T> class SliceIterator;
template <typename T> class ConstSlice;
template <typename T> class ConstSliceIterator;

template <typename T> class List : public AbstractList<T> {
public:
  using value_type = T;

  List() noexcept = default;

  explicit List(ConstSlice<T> slice) noexcept : m_vec(slice.ptr(), slice.end().ptr()) {}

  template <size_t N> explicit List(const T (&array)[N]) noexcept : List(ConstSlice(array, N)) {}

  List(std::initializer_list<T> init) : m_vec(init) {}

  SliceIterator<T> begin() noexcept {
    return SliceIterator(m_vec.data(), m_vec.size());
  }
  ConstSliceIterator<T> begin() const noexcept {
    return ConstSliceIterator(m_vec.data(), m_vec.size());
  }

  SliceIterator<T> end() noexcept {
    return SliceIterator(m_vec.data() + m_vec.size(), 0);
  }
  ConstSliceIterator<T> end() const noexcept {
    return ConstSliceIterator(m_vec.data() + m_vec.size(), 0);
  }

  Slice<T> data() noexcept {
    return Slice(m_vec.data(), m_vec.size());
  }
  ConstSlice<T> data() const noexcept {
    return ConstSlice(m_vec.data(), m_vec.size());
  }

  size_t size() const noexcept override {
    return m_vec.size();
  }

  void push_back(T value) override {
    m_vec.push_back(std::move(value));
  }

  void pop_back() {
    if (m_vec.empty()) {
      throw RuntimeError("Cannot pop_back from an empty list");
    }
    m_vec.pop_back();
  }

  void clear() noexcept {
    m_vec.clear();
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    return m_vec.emplace_back(std::forward<Args>(args)...);
  }

  void append(ConstSlice<T> slice) override {
    *this += slice;
  }

  void assign(ConstSlice<T> slice) override {
    m_vec.clear();
    *this += slice;
  }

  bool has(const T &value) const noexcept {
    for (const T &item : m_vec) {
      if (item == value) {
        return true;
      }
    }
    return false;
  }

  void reverse() noexcept {
    std::reverse(m_vec.begin(), m_vec.end());
  }

  void sort() {
    std::sort(m_vec.begin(), m_vec.end());
  }
  template <typename Compare> void sort(Compare comp) {
    std::sort(m_vec.begin(), m_vec.end(), comp);
  }

  T &operator[](size_t index) override {
    if (index >= size()) {
      throw RuntimeError("List index out of range");
    }
    return m_vec[index];
  }

  const T &operator[](size_t index) const {
    if (index >= size()) {
      throw RuntimeError("List index out of range");
    }
    return m_vec[index];
  }

  List<T> &operator+=(const List<T> &other) {
    m_vec.insert(m_vec.end(), other.m_vec.begin(), other.m_vec.end());
    return *this;
  }

  List<T> &operator+=(ConstSlice<T> slice) {
    m_vec.insert(m_vec.end(), slice.ptr(), slice.end().ptr());
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
      if (m_vec[i] != other.m_vec[i]) {
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
      if (m_vec[i] != slice[i]) {
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
  std::vector<T> m_vec;
};

} // namespace amelia
