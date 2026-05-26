#pragma once

#include "data/lexer/token.h"
#include <cstdint>

namespace amelia {

class Text;
struct NumberLiteral;

struct AbstractTokenRepository {
  virtual Token get_token(TokenId token_id) const = 0;
};

} // namespace amelia
