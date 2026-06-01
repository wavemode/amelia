#pragma once

#include <cstddef>

namespace amelia {

template <typename T> class ConstSlice;

template <typename T> struct AbstractList {
  virtual ~AbstractList() = default;

  virtual void append(ConstSlice<T> slice) = 0;
  virtual void assign(ConstSlice<T> slice) = 0;
  virtual void push_back(T value) = 0;
  virtual T &operator[](size_t index) = 0;
  virtual size_t size() const noexcept = 0;
};

} // namespace amelia
