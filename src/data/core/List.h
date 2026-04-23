#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#include "data/core/Slice.h"

#include "interface/core/IList.h"

namespace amelia {

class SliceUtils;

/**
 * @class List
 */
template <typename T> class List : public IList<T> {
public:
  List() noexcept = default;

  explicit List(Slice<T> slice) noexcept : vec(slice.ptr(), slice.end.ptr()) {}

  template <size_t N> explicit List(T (&array)[N]) noexcept : List(Slice(array, N)) {}

  List(std::initializer_list<T> init) : vec(init) {}

  Slice<T> begin() noexcept { return Slice(vec.data(), vec.size()); }
  Slice<const T> begin() const noexcept { return Slice(vec.data(), vec.size()); }

  Slice<T> end() noexcept { return Slice(vec.data() + vec.size(), 0); }
  Slice<const T> end() const noexcept { return Slice(vec.data() + vec.size(), 0); }

  size_t size() const noexcept override { return vec.size(); }

  void push_back(T value) override { vec.push_back(std::move(value)); }

  template <typename... Args> T &emplace_back(Args &&...args) {
    return vec.emplace_back(std::forward<Args>(args)...);
  }

  T &operator[](size_t index) override {
    if (index >= size()) {
      throw std::out_of_range("List index out of range");
    }
    return vec[index];
  }

  List<T> &operator+=(Slice<T> slice) {
    vec.insert(vec.end(), slice.ptr(), slice.end().ptr());
    return *this;
  }

  void append(Slice<T> slice) override { *this += slice; }

  void assign(Slice<T> slice) override {
    vec.clear();
    *this += slice;
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
      if (vec[i] != other.vec[i]) {
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
      if (vec[i] != slice[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(Slice<T> other) const noexcept { return !(*this == other); }

  void sort() { std::sort(vec.begin(), vec.end()); }
  template <typename CompareFn> void sort(CompareFn comp) {
    std::sort(vec.begin(), vec.end(), comp);
  }

  operator Slice<T>() noexcept override { return Slice(vec.data(), vec.size()); }
  operator Slice<const T>() const noexcept override { return Slice(vec.data(), vec.size()); }

  friend class SliceUtils;

private:
  std::vector<T> vec;
};

} // namespace amelia
