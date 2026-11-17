#include "token_type.hpp"

#include "util/data/abstract_string.hpp"
#include "util/data/text.hpp"

namespace amelia {
void token_type_to_string(AbstractString &out, TokenType type) {
  switch (type) {
#define X(name)                                                                \
  case TokenType::name:                                                        \
    out.append(#name);                                                         \
    break;
    TOKEN_TYPE_LIST
#undef X
  default:
    throw_runtime_error("Invalid TokenType");
  }
}

} // namespace amelia
