#pragma once

namespace amelia {

class Text;
class CharIterator;
class Token;
struct LexerContext;
struct LexerResult;
template <typename T> class AbstractList;

struct Lexer {
  static void tokenize(LexerResult &output, CharIterator &iter, LexerContext ctx);
};

} // namespace amelia
