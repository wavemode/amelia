#pragma once

namespace amelia {

class Text;
class CharIterator;
class Token;
struct LexerContext;
template <typename T> class IList;

struct Lexer {
  static void tokenize(IList<Token> &output, CharIterator &iter, LexerContext ctx);
};

} // namespace amelia
