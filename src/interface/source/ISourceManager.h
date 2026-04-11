#pragma once

#include <cstddef>
#include <cstdint>

namespace amelia {

template <typename T> class Slice;

class ISourceManager {
public:
  virtual size_t store_source(Slice<const uint32_t>) = 0;
  virtual Slice<const uint32_t> get_source(size_t) = 0;
};

} // namespace amelia
