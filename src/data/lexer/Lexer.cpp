#include <cstdint>
#include <unordered_map>

#include "Lexer.h"
#include "Prelude.h"

#include "data/lexer/LexerContext.h"
#include "data/lexer/LexerError.h"
#include "data/lexer/Location.h"
#include "data/lexer/NumberLiteral.h"
#include "data/lexer/NumberReadError.h"
#include "data/lexer/StringLiteral.h"
#include "data/lexer/Token.h"
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
    {"label", TokenType::KEYWORD_LABEL},
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

bool is_hex_digit(uint32_t cp) noexcept {
  return (cp >= '0' && cp <= '9') || (cp >= 'a' && cp <= 'f') || (cp >= 'A' && cp <= 'F');
}

struct LexerState {
  LexerContext ctx;
  size_t line;
  size_t column;
  Text file_contents;
  CharIterator input;
  IList<Token> &output;
  bool previous_char_was_whitespace;

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
      read_whitespace();
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
    } else if (cp == '~') {
      read_tilde(start_location);
    } else if (cp == '>') {
      read_greater(start_location);
    } else if (cp == '<') {
      read_less(start_location);
    } else if (cp == '{') {
      read_left_brace(start_location);
    } else if (cp == '}') {
      read_right_brace(start_location);
    } else if (cp == ';') {
      read_semicolon(start_location);
    } else if (cp == '?') {
      read_question_mark(start_location);
    } else if (cp == ',') {
      read_comma(start_location);
    } else if (cp == '.') {
      read_dot(start_location);
    } else if (cp == ':') {
      read_colon(start_location);
    } else if (cp == '(') {
      read_left_paren(start_location);
    } else if (cp == ')') {
      read_right_paren(start_location);
    } else if (cp == '[') {
      read_left_bracket(start_location);
    } else if (cp == ']') {
      read_right_bracket(start_location);
    } else if (cp == '@') {
      read_at(start_location);
    } else if (cp == '"') {
      read_quote(start_location);
    } else if (TextUtils::is_digit(cp)) {
      read_number(start_location);
    } else if (is_word_start(cp)) {
      read_word(start_location);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(start_location, std::move(msg));
    }
  }

  void read_quote(Location start_location) {
    Text first_three_chars = TextUtils::slice(file_contents, input, 3);
    if (first_three_chars == "\"\"\"") {
      read_multiline_string(start_location);
    } else {
      read_string(start_location);
    }
  }

  void read_raw_quote(Location start_location) {
    Text first_three_chars = TextUtils::slice(file_contents, input, 3);
    if (first_three_chars == "\"\"\"") {
      read_raw_multiline_string(start_location);
    } else {
      read_raw_string(start_location);
    }
  }

  void read_raw_multiline_string(Location start_location) {
    advance();
    advance();
    advance();
    auto content_start_location = current_location();
    auto content_end_location = start_location;
    size_t quotes_in_a_row = 0;
    while (!input.at_end()) {
      uint32_t cp = input.peek();
      if (cp == '"') {
        quotes_in_a_row++;
        if (quotes_in_a_row == 1) {
          content_end_location = current_location();
        }
        advance();
        if (quotes_in_a_row == 3) {
          Text content = TextUtils::substr(
              file_contents, content_start_location.position, content_end_location.position
          );
          String outcome;
          CharIterator content_iter = content.begin();
          try {
            StringLiteral::read(outcome, content_iter, true);
          } catch (const std::exception &e) {
            String msg = "Error in raw multiline string literal: ";
            msg.append(Text::from(e.what()));
            throw_lexer_error(start_location, std::move(msg));
          }
          emit_token(TokenType::RAW_MULTILINE_STRING_LITERAL, start_location);
          return;
        }
      } else {
        quotes_in_a_row = 0;
        advance();
      }
    }
    throw_lexer_error(start_location, "Unterminated raw multiline string literal");
  }

  void read_raw_string(Location start_location) {
    advance();
    auto content_start_location = current_location();
    while (!input.at_end()) {
      uint32_t cp = input.peek();
      if (cp == '"') {
        Text content = TextUtils::substr(file_contents, content_start_location.position, input);
        String outcome;
        CharIterator content_iter = content.begin();
        try {
          StringLiteral::read(outcome, content_iter, true);
        } catch (const std::exception &e) {
          String msg = "Error in raw string literal: ";
          msg.append(Text::from(e.what()));
          throw_lexer_error(start_location, std::move(msg));
        }
        advance();
        emit_token(TokenType::RAW_STRING_LITERAL, start_location);
        return;
      } else if (cp == '\n') {
        throw_lexer_error(start_location, "Unterminated raw string literal");
      } else {
        advance();
      }
    }
    throw_lexer_error(start_location, "Unterminated raw string literal");
  }

  void read_multiline_string(Location start_location) {
    advance();
    advance();
    advance();
    auto content_start_location = current_location();
    auto content_end_location = start_location;
    size_t quotes_in_a_row = 0;
    while (!input.at_end()) {
      uint32_t cp = input.peek();
      if (cp == '"') {
        quotes_in_a_row++;
        if (quotes_in_a_row == 1) {
          content_end_location = current_location();
        }
        advance();
        if (quotes_in_a_row == 3) {
          Text content = TextUtils::substr(
              file_contents, content_start_location.position, content_end_location.position
          );
          String outcome;
          CharIterator content_iter = content.begin();
          try {
            StringLiteral::read(outcome, content_iter, false);
          } catch (const std::exception &e) {
            String msg = "Error in multiline string literal: ";
            msg.append(Text::from(e.what()));
            throw_lexer_error(start_location, std::move(msg));
          }
          emit_token(TokenType::MULTILINE_STRING_LITERAL, start_location);
          return;
        }
      } else {
        quotes_in_a_row = 0;
        if (cp == '\\') {
          skip_escape_sequence();
        } else {
          advance();
        }
      }
    }
    throw_lexer_error(start_location, "Unterminated multiline string literal");
  }

  void read_string(Location start_location) {
    advance();
    auto content_start_location = current_location();
    while (!input.at_end()) {
      uint32_t cp = input.peek();
      if (cp == '\\') {
        skip_escape_sequence();
      } else if (cp == '"') {
        Text content = TextUtils::substr(file_contents, content_start_location.position, input);
        String outcome;
        CharIterator content_iter = content.begin();
        try {
          StringLiteral::read(outcome, content_iter, false);
        } catch (const std::exception &e) {
          String msg = "Error in string literal: ";
          msg.append(Text::from(e.what()));
          throw_lexer_error(start_location, std::move(msg));
        }
        advance();
        emit_token(TokenType::STRING_LITERAL, start_location);
        return;
      } else if (cp == '\n') {
        throw_lexer_error(start_location, "Unterminated string literal");
      } else {
        advance();
      }
    }
    throw_lexer_error(start_location, "Unterminated string literal");
  }

  void skip_escape_sequence() {
    advance();
    if (input.at_end()) {
      throw_lexer_error(
          current_location(), "Unexpected end of input after backslash in string literal"
      );
    }
    uint32_t cp = input.peek();
    switch (cp) {
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
    case '\\':
    case '\'':
    case '"':
    case '\n':
      advance();
      break;
    case 'x':
      advance();
      skip_hex_digits(2);
      break;
    case 'u':
      advance();
      skip_hex_digits(4);
      break;
    case 'U':
      advance();
      skip_hex_digits(8);
      break;
    default: {
      String msg = "Invalid escape sequence: '\\";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(current_location(), std::move(msg));
    }
    }
  }

  void skip_hex_digits(size_t count) {
    for (size_t i = 0; i < count; i++) {
      uint32_t cp = input.peek();
      if (!is_hex_digit(cp)) {
        String msg = "Expected ";
        TextUtils::to_string(msg, count);
        msg.append(" hex digits in escape sequence, but got '");
        msg.append(cp);
        msg.append('\'');
        throw_lexer_error(current_location(), std::move(msg));
      }
      advance();
    }
  }

  void read_at(Location start_location) {
    advance();
    skip_word_chars();
    emit_token(TokenType::ANNOTATION_NAME, start_location);
  }

  void read_right_bracket(Location start_location) {
    advance();
    emit_token(TokenType::RIGHT_BRACKET, start_location);
  }

  void read_left_bracket(Location start_location) {
    advance();
    if (previous_char_was_whitespace) {
      emit_token(TokenType::LEFT_BRACKET, start_location);
    } else {
      emit_token(TokenType::INDEXING, start_location);
    }
  }

  void read_right_paren(Location start_location) {
    advance();
    emit_token(TokenType::RIGHT_PAREN, start_location);
  }

  void read_left_paren(Location start_location) {
    advance();
    if (previous_char_was_whitespace) {
      emit_token(TokenType::LEFT_PAREN, start_location);
    } else {
      emit_token(TokenType::FUNCTION_CALL, start_location);
    }
  }

  void read_colon(Location start_location) {
    advance();
    if (input.peek() == ':' && !previous_char_was_whitespace) {
      advance();
      skip_word_chars();
      emit_token(TokenType::NAMESPACE_ACCESS, start_location);
    } else {
      emit_token(TokenType::COLON, start_location);
    }
  }

  void read_dot(Location start_location) {
    advance();
    skip_word_chars();
    if (previous_char_was_whitespace) {
      emit_token(TokenType::DOTTED_IDENTIFIER, start_location);
    } else {
      emit_token(TokenType::FIELD_ACCESS, start_location);
    }
  }

  void read_question_mark(Location start_location) {
    if (previous_char_was_whitespace) {
      throw_lexer_error(start_location, "Unexpected '?' after whitespace");
    }
    advance();
    emit_token(TokenType::QUESTION, start_location);
  }

  void read_comma(Location start_location) {
    advance();
    emit_token(TokenType::COMMA, start_location);
  }

  void read_semicolon(Location start_location) {
    advance();
    emit_token(TokenType::SEMICOLON, start_location);
  }

  void read_left_brace(Location start_location) {
    advance();
    emit_token(TokenType::LEFT_BRACE, start_location);
  }

  void read_right_brace(Location start_location) {
    advance();
    emit_token(TokenType::RIGHT_BRACE, start_location);
  }

  void read_less(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::LESS_EQUAL, start_location);
    } else {
      emit_token(TokenType::LESS, start_location);
    }
  }

  void read_greater(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::GREATER_EQUAL, start_location);
    } else {
      emit_token(TokenType::GREATER, start_location);
    }
  }

  void read_tilde(Location start_location) {
    advance();
    emit_token(TokenType::TILDE, start_location);
  }

  void read_exclamation(Location start_location) {
    advance();
    if (input.peek() == '=') {
      advance();
      emit_token(TokenType::NOT_EQUAL, start_location);
    } else if (!previous_char_was_whitespace) {
      emit_token(TokenType::EXCLAMATION, start_location);
    } else {
      emit_token(TokenType::NOT, start_location);
    }
  }

  void read_ampersand(Location start_location) {
    advance();
    uint32_t next_cp = input.peek();
    if (next_cp == '&') {
      advance();
      emit_token(TokenType::AND, start_location);
    } else if (next_cp == '=') {
      advance();
      emit_token(TokenType::AMPERSAND_EQUAL, start_location);
    } else {
      emit_token(TokenType::AMPERSAND, start_location);
    }
  }

  void read_pipe(Location start_location) {
    advance();
    uint32_t next_cp = input.peek();
    if (next_cp == '=') {
      advance();
      emit_token(TokenType::PIPE_EQUAL, start_location);
    } else if (next_cp == '|') {
      advance();
      emit_token(TokenType::OR, start_location);
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
    uint32_t next_cp = input.peek();
    if (next_cp == '=') {
      advance();
      emit_token(TokenType::MINUS_EQUAL, start_location);
    } else if (next_cp == '>') {
      advance();
      emit_token(TokenType::ARROW, start_location);
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
      throw_lexer_error(start_location, String::from(e.what()));
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

  void read_whitespace() {
    advance();
    previous_char_was_whitespace = true;
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
    if (input.peek() == 'r') {
      advance();
      if (input.peek() == '"') {
        read_raw_quote(start_location);
        return;
      }
    }
    skip_word_chars();

    Text word = TextUtils::substr(file_contents, start_location.position, input);
    auto keyword_it = keywords.find(word);

    if (keyword_it != keywords.end()) {
      emit_token(keyword_it->second, start_location);
    } else if (!input.at_end() && input.peek() == '!') {
      emit_token(TokenType::MACRO_NAME, start_location);
      advance();
    } else {
      emit_token(TokenType::IDENTIFIER, start_location);
    }
  }

  void skip_word_chars() {
    while (!input.at_end() && is_word_continue(input.peek())) {
      advance();
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

  Location current_location() const noexcept { return Location{ctx.filename, input, line, column}; }

  void emit_token(TokenType type, Location loc) { emit_token(type, loc, current_location()); }

  void emit_token(TokenType type, Location start, Location end) {
    output.push_back(
        Token{type, start, TextUtils::substr(file_contents, start.position, end.position)}
    );
    previous_char_was_whitespace = false;
  }

  void throw_lexer_error(Location loc, String message) {
    throw LexerError(loc, std::move(message));
  }
};

} // namespace

void Lexer::tokenize(IList<Token> &output, CharIterator &iter, LexerContext ctx) {
  LexerState state{
      .ctx = ctx,
      .line = 1,
      .column = 1,
      .file_contents = iter.text(),
      .input = iter,
      .output = output,
      .previous_char_was_whitespace = true
  };
  state.read_file();
}

} // namespace amelia
