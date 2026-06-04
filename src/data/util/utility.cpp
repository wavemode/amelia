#include "utility.hpp"

#include "xxhash.h"

namespace amelia {

uint64_t hash_str_64(const char *str, size_t len) {
  return XXH64(str, len, 0);
}

uint32_t hash_str_32(const char *str, size_t len) {
  return XXH32(str, len, 0);
}

} // namespace amelia
