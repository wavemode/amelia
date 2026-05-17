#pragma once

#include "data/lexer/token.h"
#include <cstdint>

namespace amelia {

class Text;
struct NumberLiteral;

struct AbstractTokenRepository {
  virtual Text string_literal_contents(TokenId token_id) const = 0;
  virtual NumberLiteral get_number_literal(TokenId token_id) const = 0;
  virtual Token get_token(TokenId token_id) const = 0;
};

} // namespace amelia
