#pragma once

#include <cstddef>

namespace amelia {

class Token;
class Text;
class NumberLiteral;

struct AbstractTokenRepository {
  virtual Text string_literal_contents(size_t token_id) const = 0;
  virtual NumberLiteral get_number_literal(size_t token_id) const = 0;
  virtual Token get_token(size_t token_id) const = 0;
};

} // namespace amelia
