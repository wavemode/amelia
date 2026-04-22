#pragma once

#include <cstddef>
#include <stdexcept>

namespace amelia {

/**
 * @class Slice
 */
template <typename T> class Slice {
public:
  Slice() noexcept : data_ptr(nullptr), length(0) {}

  Slice(T *data_ptr, size_t length) noexcept : data_ptr(data_ptr), length(length) {}

  template <size_t N> explicit Slice(T (&array)[N]) noexcept : data_ptr(array), length(N) {}

  Slice<T> begin() const { return *this; }

  Slice<T> end() const { return Slice(data_ptr + length, 0); }

  T *ptr() const noexcept { return data_ptr; }

  size_t size() const noexcept { return length; }

  T &operator[](size_t index) const {
    if (index >= length) {
      throw std::out_of_range("Slice index out of range");
    }
    return data_ptr[index];
  }

  T &operator*() const {
    if (length == 0) {
      throw std::out_of_range("Dereferencing end of slice");
    }

    return *data_ptr;
  }

  Slice<T> &operator+=(size_t offset) {
    *this = *this + offset;
    return *this;
  }

  Slice<T> &operator++() {
    *this += 1;
    return *this;
  }

  Slice<T> operator++(int) {
    Slice<T> temp = *this;
    ++(*this);
    return temp;
  }

  Slice<T> operator+(size_t offset) const {
    if (offset > length) {
      throw std::out_of_range("Slice offset out of range");
    }
    return Slice(data_ptr + offset, length - offset);
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

private:
  T *data_ptr;
  size_t length;
};

} // namespace amelia
