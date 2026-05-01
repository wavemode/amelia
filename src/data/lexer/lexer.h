#pragma once

namespace amelia {

class Text;
class CharIterator;
class Token;
struct LexerContext;
template <typename T> class AbstractList;

struct Lexer {
  static void tokenize(AbstractList<Token> &output, CharIterator &iter, LexerContext ctx);
};

} // namespace amelia
