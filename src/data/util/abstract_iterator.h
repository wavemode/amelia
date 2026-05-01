#pragma once

#include <cstddef>

namespace amelia {

template <typename T> class Slice;

template <typename T> struct AbstractIterator {
  virtual ~AbstractIterator() = default;

  virtual T peek() = 0;
  virtual T next() = 0;
  virtual bool at_end() const = 0;
};

} // namespace amelia
