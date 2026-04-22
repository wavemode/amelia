#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#include "data/core/Slice.h"

namespace amelia {

/**
 * @class List
 */
template <typename T> class List {
public:
  List() noexcept = default;

  explicit List(Slice<T> slice) noexcept : data(slice.ptr(), slice.end.ptr()) {}

  template <size_t N> explicit List(T (&array)[N]) noexcept : List(Slice(array, N)) {}

  List(std::initializer_list<T> init) : data(init) {}

  Slice<T> begin() noexcept { return Slice(data.data(), data.size()); }
  Slice<const T> begin() const noexcept { return Slice(data.data(), data.size()); }

  Slice<T> end() noexcept { return Slice(data.data() + data.size(), 0); }
  Slice<const T> end() const noexcept { return Slice(data.data() + data.size(), 0); }

  size_t size() const noexcept { return data.size(); }

  void push_back(const T &value) { data.push_back(value); }

  template <typename... Args> T &emplace_back(Args &&...args) {
    return data.emplace_back(std::forward<Args>(args)...);
  }

  T &operator[](size_t index) {
    if (index >= size()) {
      throw std::out_of_range("List index out of range");
    }
    return data[index];
  }

  List<T> &operator+=(Slice<T> slice) {
    data.insert(data.end(), slice.ptr(), slice.end().ptr());
    return *this;
  }

  List<T> operator+(Slice<T> slice) const {
    List<T> result = *this;
    result += slice;
    return result;
  }

  bool operator==(const List<T> &other) const noexcept {
    if (size() != other.size()) {
      return false;
    }
    for (size_t i = 0; i < size(); ++i) {
      if (data[i] != other.data[i]) {
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
      if (data[i] != slice[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(Slice<T> other) const noexcept { return !(*this == other); }

  void sort() { std::sort(data.begin(), data.end()); }
  template <typename CompareFn> void sort(CompareFn comp) {
    std::sort(data.begin(), data.end(), comp);
  }

  operator Slice<T>() noexcept { return Slice(data.data(), data.size()); }
  operator Slice<const T>() const noexcept { return Slice(data.data(), data.size()); }

private:
  std::vector<T> data;
};

} // namespace amelia
