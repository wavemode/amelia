#include "Lexer.h"

#include <cstdint>

#include "data/lexer/LexerContext.h"
#include "data/lexer/LexerError.h"
#include "data/source/Location.h"
#include "data/source/Token.h"
#include "util/text/CharIterator.h"
#include "util/text/Text.h"

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

struct LexerState {
  LexerContext ctx;
  size_t line;
  size_t column;
  size_t position;
  CharIterator input;
  std::vector<Token> &output;

  void read_file() {
    while (!input.at_end()) {
      auto slice_start = input.position();
      auto start_location = current_location();
      TokenType token_type;

      uint32_t cp = advance();
      if (cp == '=') {
        if (input.peek() == '=') {
          advance();
          token_type = TokenType::EQUAL;
        } else {
          token_type = TokenType::ASSIGN;
        }
      } else if (is_whitespace(cp)) {
        continue;
      } else if (cp == '/' && input.peek() == '/') {
        advance();
        read_single_line_comment();
        continue;
      } else if (cp == '/' && input.peek() == '*') {
        advance();
        read_multi_line_comment();
        continue;
      } else if (is_ident_start(cp)) {
        read_identifier();
        token_type = TokenType::IDENTIFIER;
      } else {
        String msg("Unexpected character: ");
        msg.append(cp);
        throw_lexer_error(msg);
      }

      auto slice_end = input.position();
      emit_token(token_type, start_location, input.slice(slice_start, slice_end));
    }

    auto eof_position = input.position();
    emit_token(TokenType::END_OF_FILE, current_location(), input.slice(eof_position, eof_position));
  }

  void read_identifier() {
    while (!input.at_end() && is_ident_continue(input.peek())) {
      advance();
    }
  }

  void read_single_line_comment() {
    while (!input.at_end() && advance() != '\n') {
      // skip
    }
  }

  void read_multi_line_comment() {
    while (!input.at_end()) {
      uint32_t cp = advance();
      if (cp == '/' && input.peek() == '*') {
        advance();
        read_multi_line_comment();
      } else if (cp == '*' && input.peek() == '/') {
        advance();
        break;
      }
    }
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

  void emit_token(TokenType type, Location loc, Text contents) {
    output.push_back(Token{type, loc, contents});
  }

  void throw_lexer_error(String message) { throw LexerError(current_location(), message); }
};

} // namespace

void Lexer::tokenize(LexerContext ctx, Text input, std::vector<Token> &output) {
  LexerState state{ctx, 1, 1, 0, CharIterator(input), output};
  state.read_file();
}

} // namespace amelia
