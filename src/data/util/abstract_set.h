#pragma once

#include <cstddef>

namespace amelia {
template <typename T> struct AbstractSet {
  virtual ~AbstractSet() = default;

  virtual void add(T value) = 0;
  virtual bool has(const T &value) const = 0;
  virtual void remove(const T &value) = 0;
  virtual void clear() = 0;
  virtual size_t size() const noexcept = 0;
};

} // namespace amelia
