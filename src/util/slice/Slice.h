#pragma once

#include <cstddef>
#include <exception>

namespace amelia {

/**
 * @class Slice
 */
template <typename T> class Slice {
public:
  Slice(T *data, size_t size) noexcept : data(data), size(size) {}

  template <size_t N> Slice(T (&array)[N]) noexcept : data(array), size(N) {}

  class Iterator {
  public:
    Iterator(const Slice<T> &slice) : slice(slice) {}

    T &operator*() const { return slice[0]; }
    Iterator<T> &operator++() {
      slice += 1;
      return *this;
    }
    Iterator<T> operator+(size_t offset) const { return Iterator(slice + offset); }
    bool operator==(const Iterator<T> &other) const noexcept {
      return slice.data == other.slice.data;
    }
    bool operator!=(const Iterator<T> &other) const noexcept { return !(*this == other); }

    bool at_end() const noexcept { return slice.size == 0; }

  private:
    const Slice slice;
  };

  Iterator<T> begin() const { return Iterator(*this); }
  Iterator<T> end() const {
    return Iterator(Slice(data + size,

                          0));
  }

  size_t size() const noexcept { return size; }

  T &operator[](size_t index) const {
    if (index >= size) {
      throw std::out_of_range("Slice index out of range");
    }
    return data[index];
  }
  Slice<T> &operator++() {
    *this += 1;
    return *this;
  }

  Slice<T> operator+(size_t offset) const {
    if (offset > size) {
      throw std::out_of_range("Slice offset out of range");
    }
    return Slice(data + offset, size - offset);
  }

  bool operator==(const Slice<T> &other) const noexcept {
    return data == other.data && size == other.size;
  }
  bool operator!=(const Slice<T> &other) const noexcept { return !(*this == other); }

private:
  T *data;
  size_t size;
};

} // namespace amelia
