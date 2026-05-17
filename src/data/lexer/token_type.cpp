#include "token_type.h"
#include "prelude.h"

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
    throw std::runtime_error("Invalid TokenType");
  }
}

} // namespace amelia
