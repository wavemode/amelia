#include "TokenId.h"

amelia::TokenId::TokenId(uint64_t id) : id(id) {}

bool amelia::TokenId::operator==(const TokenId &other) const { return id == other.id; }

bool amelia::TokenId::operator!=(const TokenId &other) const { return !(*this == other); }

size_t amelia::TokenId::hash() const { return std::hash<uint64_t>()(id); }
