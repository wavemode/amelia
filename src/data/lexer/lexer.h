#pragma once

namespace amelia {

class Text;
struct LexerContext;
struct LexerResult;
struct AbstractString;
struct NumberLiteral;

struct Lexer {
  static void tokenize(LexerResult &output, LexerContext ctx, Text input);
  static void read_string_literal(AbstractString &out, Text input, bool escape = false);
  static void read_char_literal(AbstractString &out, Text input, bool escape = false);
  static NumberLiteral read_number_literal(Text input);
};

} // namespace amelia
