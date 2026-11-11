#pragma once

#include <cstdint>

#include "lexer/data/token_type.hpp"
#include "util/data/map.hpp"
#include "util/data/text.hpp"

namespace amelia {

class Identifier;
struct NumberLiteral;
struct LexerResult;
struct LexerContext;

extern const Map<Text, TokenType> RESERVED_WORDS;

struct Lexer {
  static void tokenize(LexerResult &output, LexerContext ctx, Text input);
  static void read_string_literal_contents(AbstractString &out, Text input,
                                           bool raw = false);
  static uint32_t read_char_literal_contents(Text input);
  static void read_number_literal_contents(NumberLiteral &result, Text input);
};

} // namespace amelia
