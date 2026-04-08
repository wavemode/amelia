#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace amelia {

class TokenId {
public:
  explicit TokenId(uint64_t id);

  bool operator==(const TokenId &other) const;
  bool operator!=(const TokenId &other) const;
  size_t hash() const;

private:
  uint64_t id;
};

} // namespace amelia

namespace std {
template <> struct hash<amelia::TokenId> {
  size_t operator()(const amelia::TokenId &token_id) const { return token_id.hash(); }
};
} // namespace std
