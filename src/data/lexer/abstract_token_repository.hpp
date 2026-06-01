#pragma once

#include "data/lexer/token.hpp"
#include <cstdint>

namespace amelia {

class Text;
struct NumberLiteral;
using TokenId = int32_t;

struct AbstractTokenRepository {
  virtual Token get_token(TokenId token_id) const = 0;
};

} // namespace amelia
