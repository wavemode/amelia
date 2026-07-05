#include "token_type.hpp"
#include "prelude.hpp"

namespace amelia {
void token_type_to_string(AbstractString &out, TokenType type) {
  switch (type) {
#define X(name)                                                                                    \
  case TokenType::name:                                                                            \
    out.append(#name);                                                                             \
    break;
    TOKEN_TYPE_LIST
#undef X
  default:
    throw RuntimeError("Invalid TokenType");
  }
}

} // namespace amelia
