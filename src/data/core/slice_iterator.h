#pragma once

#include <cstddef>

namespace amelia {

template <typename T> class Slice;
class RuntimeError;

/**
 * @class SliceIterator
 */
template <typename T> class SliceIterator {
public:
  SliceIterator() noexcept : data_ptr(nullptr), length(0) {}

  SliceIterator(Slice<T> slice) noexcept : data_ptr(slice.ptr()), length(slice.size()) {}

  T *ptr() const noexcept { return data_ptr; }

  size_t size() const noexcept { return length; }

  SliceIterator<T> begin() const { return *this; }
  SliceIterator<T> end() const { return Slice(data_ptr + length, 0); }

  T &operator[](size_t index) const {
    if (index >= length) {
      throw RuntimeError("SliceIterator index out of range");
    }
    return data_ptr[index];
  }

  bool operator==(const SliceIterator<T> &other) const noexcept {
    return data_ptr == other.data_ptr;
  }

  bool operator!=(const SliceIterator<T> &other) const noexcept { return !(*this == other); }

  T &operator*() const {
    if (length == 0) {
      throw RuntimeError("Dereferencing end of slice");
    }

    return *data_ptr;
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
    Slice<T> temp = *this;
    ++(*this);
    return temp;
  }

  SliceIterator<T> operator+(size_t offset) const {
    if (offset > length) {
      throw RuntimeError("Slice iterator offset out of range");
    }
    return Slice(data_ptr + offset, length - offset);
  }

private:
  T *data_ptr;
  size_t length;
};

} // namespace amelia
