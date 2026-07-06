#pragma once

#include <cstdint>

namespace amelia {

struct Token;
class Text;
struct NumberLiteral;
using TokenId = int32_t;

struct AbstractTokenRepository {
  virtual const Token &get_token(TokenId token_id) const = 0;
};

} // namespace amelia
