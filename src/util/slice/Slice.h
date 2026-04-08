#pragma once

#include <cstddef>
#include <stdexcept>

namespace amelia {

/**
 * @class Slice
 */
template <typename T> class Slice {
public:
  class Iterator {
  public:
    Iterator(const Slice<T> &slice) : slice(slice) {}

    T &operator*() const { return slice[0]; }

    Iterator &operator+=(size_t offset) {
      *this = *this + offset;
      return *this;
    }

    Iterator &operator++() {
      slice += 1;
      return *this;
    }

    Iterator operator+(size_t offset) const { return Iterator(slice + offset); }

    bool operator==(const Iterator &other) const noexcept { return slice.data == other.slice.data; }

    bool operator!=(const Iterator &other) const noexcept { return !(*this == other); }

    bool at_end() const noexcept { return slice.length == 0; }

  private:
    Slice slice;
  };

  Slice(T *data, size_t length) noexcept : data(data), length(length) {}

  template <size_t N> explicit Slice(T (&array)[N]) noexcept : data(array), length(N) {}

  Iterator begin() const { return Iterator(*this); }

  Iterator end() const { return Iterator(Slice(data + length, 0)); }

  size_t size() const noexcept { return length; }

  T &operator[](size_t index) const {
    if (index >= length) {
      throw std::out_of_range("Slice index out of range");
    }
    return data[index];
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
    return Slice(data + offset, length - offset);
  }

  bool operator==(const Slice<T> &other) const noexcept {
    if (length != other.length) {
      return false;
    }
    for (size_t i = 0; i < length; ++i) {
      if (data[i] != other.data[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Slice<T> &other) const noexcept { return !(*this == other); }

private:
  T *data;
  size_t length;
};

} // namespace amelia
