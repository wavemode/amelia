#pragma once

#include <cstdint>

#include "prelude.hpp"

#include "data/lexer/lexer_context.hpp"
#include "data/lexer/lexer_result.hpp"
#include "data/source/number_literal.hpp"

namespace amelia {

struct Lexer {
  static void tokenize(LexerResult &output, LexerContext ctx, Text input);
  static void read_string_literal(AbstractString &out, Text input, bool escape = false);
  static void read_quoted_ident(AbstractString &out, Text input, bool escape = false);
  static uint32_t read_char_literal(Text input);
  static NumberLiteral read_number_literal(Text input);
};

} // namespace amelia
