#include "Lexer.h"
#include "Prelude.h"

#include <cstdint>

#include "data/lexer/LexerContext.h"
#include "data/lexer/LexerError.h"
#include "data/source/Location.h"
#include "data/source/Token.h"
#include "data/text/TextUtils.h"

namespace amelia {

namespace {

bool is_whitespace(uint32_t cp) noexcept {
  return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r';
}

bool is_ident_start(uint32_t cp) noexcept {
  return (cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z') || cp == '_';
}

bool is_ident_continue(uint32_t cp) noexcept {
  return is_ident_start(cp) || (cp >= '0' && cp <= '9');
}

bool is_number_continue(uint32_t cp) noexcept {
  return (
      // digit
      (cp >= '0' && cp <= '9')
      // decimal separator
      || cp == '.'
      // digit separator
      || cp == '_'
             // base prefixes
             | cp == 'x' ||
      cp == 'X' || cp == 'b' || cp == 'B' || cp == 'o' ||
      cp == 'O'
      // hex digits
      || (cp >= 'a' && cp <= 'f') || (cp >= 'A' && cp <= 'F')
  );
}

bool is_exponent_start(uint32_t cp) noexcept { return cp == 'e' || cp == 'E'; }

bool is_sign(uint32_t cp) noexcept { return cp == '+' || cp == '-'; }

struct LexerState {
  LexerContext ctx;
  size_t line;
  size_t column;
  size_t position;
  Text file_contents;
  CharIterator input;
  IList<Token> &output;

  void read_file() {
    while (!input.at_end()) {
      read_token();
    }

    emit_token(TokenType::END_OF_FILE, current_location(), input);
  }

  void read_token() {
    uint32_t cp = input.peek();
    auto start_location = current_location();
    auto content_start = input;
    if (is_whitespace(cp)) {
      advance();
    } else if (cp == '=') {
      read_equal(start_location, content_start);
    } else if (cp == '/') {
      read_slash(start_location, content_start);
    } else if (TextUtils::is_digit(cp)) {
      read_number(start_location, content_start);
    } else if (is_ident_start(cp)) {
      read_identifier(start_location, content_start);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(std::move(msg));
    }
  }

  void read_number(Location start_location, CharIterator content_start) {}

  void read_equal(Location start_location, CharIterator content_start) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::EQUAL, start_location, content_start);
    } else {
      emit_token(TokenType::ASSIGN, start_location, content_start);
    }
  }

  void read_slash(Location start_location, CharIterator content_start) {
    advance();
    if (input.peek() == '/') {
      advance();
      skip_single_line_comment();
    } else if (input.peek() == '*') {
      advance();
      skip_multi_line_comment();
    } else {
      throw_lexer_error("not implemented");
    }
  }

  void skip_single_line_comment() {
    advance();
    advance();
    while (!input.at_end() && advance() != '\n') {
      // skip
    }
  }

  void skip_multi_line_comment() {
    advance();
    advance();
    while (!input.at_end()) {
      uint32_t cp = advance();
      if (cp == '*' && input.peek() == '/') {
        advance();
        break;
      }
    }
  }

  void read_identifier(Location start_location, CharIterator content_start) {
    while (!input.at_end() && is_ident_continue(input.peek())) {
      advance();
    }
    emit_token(TokenType::IDENTIFIER, start_location, content_start);
  }

  uint32_t advance() noexcept {
    uint32_t cp = input.next();
    if (cp == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
    return cp;
  }

  Location current_location() const noexcept { return Location{ctx.filename, line, column}; }

  void emit_token(TokenType type, Location loc, CharIterator content_start) {
    emit_token(type, loc, content_start, input);
  }

  void emit_token(
      TokenType type, Location loc, CharIterator content_start, CharIterator content_end
  ) {
    output.push_back(Token{type, loc, TextUtils::substr(file_contents, content_start, content_end)}
    );
  }

  void throw_lexer_error(String message) {
    throw LexerError(current_location(), std::move(message));
  }
};

} // namespace

void Lexer::tokenize(IList<Token> &output, CharIterator &iter, LexerContext ctx) {
  LexerState state{ctx, 1, 1, 0, iter.text(), iter, output};
  state.read_file();
}

} // namespace amelia
