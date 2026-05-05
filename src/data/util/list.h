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

template <typename T> class List : public AbstractList<T> {
public:
  using value_type = T;

  List() noexcept = default;

  explicit List(Slice<T> slice) noexcept : m_vec(slice.ptr(), slice.end().ptr()) {}

  template <size_t N> explicit List(T (&array)[N]) noexcept : List(Slice(array, N)) {}

  List(std::initializer_list<T> init) : m_vec(init) {}

  SliceIterator<T> begin() noexcept { return Slice(m_vec.data(), m_vec.size()).begin(); }
  SliceIterator<const T> begin() const noexcept {
    return Slice(m_vec.data(), m_vec.size()).begin();
  }

  SliceIterator<T> end() noexcept { return Slice(m_vec.data() + m_vec.size(), 0).end(); }
  SliceIterator<const T> end() const noexcept {
    return Slice(m_vec.data() + m_vec.size(), 0).end();
  }

  Slice<const T> data() const noexcept { return Slice(m_vec.data(), m_vec.size()); }

  size_t size() const noexcept override { return m_vec.size(); }

  void push_back(T value) override { m_vec.push_back(std::move(value)); }

  void pop_back() {
    if (m_vec.empty()) {
      throw RuntimeError("Cannot pop_back from an empty list");
    }
    m_vec.pop_back();
  }

  void clear() noexcept { m_vec.clear(); }

  template <typename... Args> T &emplace_back(Args &&...args) {
    return m_vec.emplace_back(std::forward<Args>(args)...);
  }

  void append(Slice<T> slice) override { *this += slice; }

  void append(Slice<const T> slice) { *this += slice; }

  void assign(Slice<T> slice) override {
    m_vec.clear();
    *this += slice;
  }

  void assign(Slice<const T> slice) {
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

  void reverse() noexcept { std::reverse(m_vec.begin(), m_vec.end()); }

  void sort() { std::sort(m_vec.begin(), m_vec.end()); }
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

  List<T> &operator+=(Slice<T> slice) {
    m_vec.insert(m_vec.end(), slice.ptr(), slice.end().ptr());
    return *this;
  }

  List<T> &operator+=(Slice<const T> slice) {
    m_vec.insert(m_vec.end(), slice.ptr(), slice.end().ptr());
    return *this;
  }

  List<T> operator+(Slice<T> slice) const {
    List<T> result = *this;
    result += slice;
    return result;
  }

  List<T> operator+(Slice<const T> slice) const {
    List<T> result = *this;
    result += slice;
    return result;
  }

  bool operator==(const List<T> &other) const noexcept {
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
  bool operator!=(const List<T> &other) const noexcept { return !(*this == other); }

  bool operator==(Slice<T> slice) const noexcept {
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
  bool operator!=(Slice<T> other) const noexcept { return !(*this == other); }

  operator Slice<T>() noexcept override { return Slice(m_vec.data(), m_vec.size()); }
  operator Slice<const T>() const noexcept override { return Slice(m_vec.data(), m_vec.size()); }

private:
  std::vector<T> m_vec;
};

} // namespace amelia
