#pragma once

#include <cstddef>

namespace amelia {
template <typename K, typename V> struct AbstractMap {
  virtual ~AbstractMap() = default;

  virtual bool has(const K &key) const = 0;
  virtual void set(const K &key, V value) = 0;
  virtual V &get(const K &key) = 0;
  virtual const V &get(const K &key) const = 0;
  virtual void remove(const K &key) = 0;
  virtual void clear() = 0;
  virtual size_t size() const noexcept = 0;
};

} // namespace amelia
