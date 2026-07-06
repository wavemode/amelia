#pragma once

#include <cstdint>

#include "prelude.hpp"

#include "util/data/map.hpp"
#include "lexer/data/token_type.hpp"

namespace amelia {

class Identifier;
struct NumberLiteral;
struct LexerResult;
struct LexerContext;

extern const Map<Text, TokenType> RESERVED_WORDS;

struct Lexer {
  static void tokenize(LexerResult &output, LexerContext ctx, Text input);
  static void read_string_literal(AbstractString &out, Text input, bool escape = false);
  static Identifier read_quoted_ident(Text input);
  static uint32_t read_char_literal(Text input);
  static NumberLiteral read_number_literal(Text input);
};

} // namespace amelia
