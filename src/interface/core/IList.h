#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Slice;

template <typename T> class IList {
public:
  virtual ~IList() = default;

  virtual void append(Slice<T> slice) = 0;
  virtual void assign(Slice<T> slice) = 0;
  virtual void push_back(T value) = 0;
  virtual T &operator[](size_t index) = 0;
  virtual size_t size() const noexcept = 0;
  virtual operator Slice<T>() noexcept = 0;
  virtual operator Slice<const T>() const noexcept = 0;
};

} // namespace amelia
