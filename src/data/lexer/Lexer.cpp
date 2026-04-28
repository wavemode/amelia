#include <cstdint>
#include <unordered_map>

#include "Lexer.h"
#include "Prelude.h"

#include "data/lexer/LexerContext.h"
#include "data/lexer/LexerError.h"
#include "data/lexer/NumberLiteral.h"
#include "data/lexer/NumberReadError.h"
#include "data/source/Location.h"
#include "data/source/Token.h"
#include "data/text/TextUtils.h"

namespace amelia {

namespace {

const std::unordered_map<Text, TokenType> keywords = {
    {"fun", TokenType::KEYWORD_FUN},
    {"if", TokenType::KEYWORD_IF},
    {"else", TokenType::KEYWORD_ELSE},
    {"try", TokenType::KEYWORD_TRY},
    {"catch", TokenType::KEYWORD_CATCH},
    {"static", TokenType::KEYWORD_STATIC},
    {"this", TokenType::KEYWORD_THIS},
    {"module", TokenType::KEYWORD_MODULE},
    {"void", TokenType::KEYWORD_VOID},
    {"throw", TokenType::KEYWORD_THROW},
    {"import", TokenType::KEYWORD_IMPORT},
    {"as", TokenType::KEYWORD_AS},
    {"switch", TokenType::KEYWORD_SWITCH},
    {"case", TokenType::KEYWORD_CASE},
    {"class", TokenType::KEYWORD_CLASS},
    {"union", TokenType::KEYWORD_UNION},
    {"record", TokenType::KEYWORD_RECORD},
    {"type", TokenType::KEYWORD_TYPE},
    {"concept", TokenType::KEYWORD_CONCEPT},
    {"bool", TokenType::KEYWORD_BOOL},
    {"auto", TokenType::KEYWORD_AUTO},
    {"let", TokenType::KEYWORD_LET},
    {"const", TokenType::KEYWORD_CONST},
    {"impl", TokenType::KEYWORD_IMPL},
    {"any", TokenType::KEYWORD_ANY},
    {"goto", TokenType::KEYWORD_GOTO},
    {"async", TokenType::KEYWORD_ASYNC},
    {"await", TokenType::KEYWORD_AWAIT},
    {"true", TokenType::KEYWORD_TRUE},
    {"false", TokenType::KEYWORD_FALSE},
    {"null", TokenType::KEYWORD_NULL},
    {"default", TokenType::KEYWORD_DEFAULT},
    {"open", TokenType::KEYWORD_OPEN},
    {"override", TokenType::KEYWORD_OVERRIDE},
    {"local", TokenType::KEYWORD_LOCAL},
    {"public", TokenType::KEYWORD_PUBLIC},
    {"private", TokenType::KEYWORD_PRIVATE},
    {"protected", TokenType::KEYWORD_PROTECTED},
    {"enum", TokenType::KEYWORD_ENUM},
    {"copy", TokenType::KEYWORD_COPY},
    {"move", TokenType::KEYWORD_MOVE},
    {"operator", TokenType::KEYWORD_OPERATOR},
    {"extern", TokenType::KEYWORD_EXTERN},
    {"inline", TokenType::KEYWORD_INLINE},
    {"delete", TokenType::KEYWORD_DELETE},
    {"new", TokenType::KEYWORD_NEW},
    {"implicit", TokenType::KEYWORD_IMPLICIT},
    {"with", TokenType::KEYWORD_WITH},
    {"when", TokenType::KEYWORD_WHEN},
    {"return", TokenType::KEYWORD_RETURN},
    {"continue", TokenType::KEYWORD_CONTINUE},
    {"break", TokenType::KEYWORD_BREAK},
    {"while", TokenType::KEYWORD_WHILE},
    {"for", TokenType::KEYWORD_FOR},
    {"in", TokenType::KEYWORD_IN},
};

bool is_whitespace(uint32_t cp) noexcept {
  return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r';
}

bool is_word_start(uint32_t cp) noexcept {
  return (cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z') || cp == '_';
}

bool is_word_continue(uint32_t cp) noexcept {
  return is_word_start(cp) || (cp >= '0' && cp <= '9');
}

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

    emit_token(TokenType::END_OF_FILE, current_location());
  }

  void read_token() {
    uint32_t cp = input.peek();
    auto start_location = current_location();
    if (is_whitespace(cp)) {
      advance();
    } else if (cp == '=') {
      read_equal(start_location);
    } else if (cp == '/') {
      read_slash(start_location);
    } else if (cp == '+') {
      read_plus(start_location);
    } else if (cp == '-') {
      read_minus(start_location);
    } else if (cp == '*') {
      read_star(start_location);
    } else if (cp == '%') {
      read_percent(start_location);
    } else if (cp == '^') {
      read_caret(start_location);
    } else if (cp == '|') {
      read_pipe(start_location);
    } else if (cp == '&') {
      read_ampersand(start_location);
    } else if (cp == '!') {
      read_exclamation(start_location);
    } else if (TextUtils::is_digit(cp)) {
      read_number(start_location);
    } else if (is_word_start(cp)) {
      read_word(start_location);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(std::move(msg));
    }
  }

  void read_exclamation(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::NOT_EQUAL, start_location);
    } else {
      emit_token(TokenType::NOT, start_location);
    }
  }

  void read_ampersand(Location start_location) {
    advance();
    if (input.peek() == '&') {
      advance();
      emit_token(TokenType::AND, start_location);
    } else if (input.peek() == '=') {
      advance();
      emit_token(TokenType::AMPERSAND_EQUAL, start_location);
    } else {
      emit_token(TokenType::AMPERSAND, start_location);
    }
  }

  void read_pipe(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::PIPE_EQUAL, start_location);
    } else {
      emit_token(TokenType::PIPE, start_location);
    }
  }

  void read_caret(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::CARET_EQUAL, start_location);
    } else {
      emit_token(TokenType::CARET, start_location);
    }
  }

  void read_percent(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::PERCENT_EQUAL, start_location);
    } else {
      emit_token(TokenType::PERCENT, start_location);
    }
  }

  void read_star(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::STAR_EQUAL, start_location);
    } else {
      emit_token(TokenType::STAR, start_location);
    }
  }

  void read_minus(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::MINUS_EQUAL, start_location);
    } else {
      emit_token(TokenType::MINUS, start_location);
    }
  }

  void read_plus(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::PLUS_EQUAL, start_location);
    } else {
      emit_token(TokenType::PLUS, start_location);
    }
  }

  void read_number(Location start_location) {
    auto it = input;
    try {
      NumberLiteral::read(it);
    } catch (const NumberReadError &e) {
      throw_lexer_error(String::from(e.what()));
    }
    size_t chars_advanced = it.data().ptr() - input.data().ptr();
    for (size_t i = 0; i < chars_advanced; ++i) {
      advance();
    }
    emit_token(TokenType::NUMBER, start_location);
  }

  void read_equal(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::EQUAL, start_location);
    } else {
      emit_token(TokenType::ASSIGN, start_location);
    }
  }

  void read_slash(Location start_location) {
    advance();
    uint32_t next_cp = input.peek();
    if (next_cp == '/') {
      advance();
      skip_until_end_of_single_line_comment();
    } else if (next_cp == '*') {
      advance();
      skip_until_end_of_multiline_comment();
    } else if (next_cp == '=') {
      advance();
      emit_token(TokenType::SLASH_EQUAL, start_location);
    } else {
      emit_token(TokenType::SLASH, start_location);
    }
  }

  void skip_until_end_of_single_line_comment() {
    while (!input.at_end() && advance() != '\n') {
      // skip
    }
  }

  void skip_until_end_of_multiline_comment() {
    uint32_t prev = 0;
    uint32_t cp = 0;
    while (!input.at_end()) {
      prev = cp;
      cp = advance();
      if (prev == '*' && cp == '/') {
        break;
      }
    }
  }

  void read_word(Location start_location) {
    while (!input.at_end() && is_word_continue(input.peek())) {
      advance();
    }
    Text word = TextUtils::substr(file_contents, start_location.position, input);
    auto keyword_it = keywords.find(word);
    TokenType tt;
    if (keyword_it != keywords.end()) {
      tt = keyword_it->second;
    } else {
      tt = TokenType::IDENTIFIER;
    }
    output.push_back(Token{tt, start_location, word});
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

  Location current_location() const noexcept { return Location{ctx.filename, input, line, column}; }

  void emit_token(TokenType type, Location loc) { emit_token(type, loc, current_location()); }

  void emit_token(TokenType type, Location start, Location end) {
    output.push_back(
        Token{type, start, TextUtils::substr(file_contents, start.position, end.position)}
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
