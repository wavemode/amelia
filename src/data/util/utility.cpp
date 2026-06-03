#include "utility.hpp"

#include "xxhash.h"

namespace amelia {

uint64_t hash_str(const char *str, size_t len) {
  return XXH64(str, len, 0);
}

} // namespace amelia
