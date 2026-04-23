#pragma once

#include <cstddef>
#include <stdexcept>

#include "SliceIterator.h"

namespace amelia {

class ListUtils;

/**
 * @class Slice
 */
template <typename T> class Slice {
public:
  Slice() noexcept : data_ptr(nullptr), length(0) {}

  Slice(T *data_ptr, size_t length) noexcept : data_ptr(data_ptr), length(length) {}

  template <size_t N> explicit Slice(T (&array)[N]) noexcept : data_ptr(array), length(N) {}

  explicit Slice(SliceIterator<T> iter) noexcept : data_ptr(iter.ptr()), length(iter.size()) {}

  SliceIterator<T> begin() const { return SliceIterator(*this); }

  SliceIterator<T> end() const { return SliceIterator(Slice(data_ptr + length, 0)); }

  T *ptr() const noexcept { return data_ptr; }

  size_t size() const noexcept { return length; }

  T &operator[](size_t index) const {
    if (index >= length) {
      throw std::out_of_range("Slice index out of range");
    }
    return data_ptr[index];
  }

  bool operator==(const Slice<T> &other) const noexcept {
    if (length != other.length) {
      return false;
    }
    if (data_ptr == other.data_ptr) {
      return true;
    }
    for (size_t i = 0; i < length; ++i) {
      if (data_ptr[i] != other.data_ptr[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Slice<T> &other) const noexcept { return !(*this == other); }

  friend class ListUtils;

private:
  T *data_ptr;
  size_t length;
};

} // namespace amelia
