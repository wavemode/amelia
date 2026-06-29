#include <cstdio>

#include "char_literal.hpp"

namespace amelia {

Serialize serialize_char_literal(uint32_t code_point, bool quoted) {
  String repr;
  if (quoted) {
    repr.append('\'');
  }
  if (code_point == '\\') {
    repr.append('\\');
    repr.append('\\');
  } else if (code_point == '\'') {
    repr.append('\\');
    repr.append('\'');
  } else if (code_point == '\a') {
    repr.append('\\');
    repr.append('a');
  } else if (code_point == '\b') {
    repr.append('\\');
    repr.append('b');
  } else if (code_point == '\f') {
    repr.append('\\');
    repr.append('f');
  } else if (code_point == '\n') {
    repr.append('\\');
    repr.append('n');
  } else if (code_point == '\r') {
    repr.append('\\');
    repr.append('r');
  } else if (code_point == '\t') {
    repr.append('\\');
    repr.append('t');
  } else if (code_point == '\v') {
    repr.append('\\');
    repr.append('v');
  } else if (code_point < 32) {
    repr.append('\\');
    repr.append('x');
    char hex_digits[3];
    snprintf(hex_digits, 3, "%02x", code_point);
    repr.append(hex_digits);
  } else if (code_point > 126) {
    repr.append('\\');
    repr.append('U');
    char hex_digits[9];
    snprintf(hex_digits, 9, "%08x", code_point);
    repr.append(hex_digits);
  } else {
    repr.append(code_point);
  }
  if (quoted) {
    repr.append('\'');
  }
  return Serialize::literal(move(repr));
}

} // namespace amelia
