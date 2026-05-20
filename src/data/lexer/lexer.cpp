#include <cstdint>

#include "lexer.h"
#include "prelude.h"

#include "data/lexer/lexer_context.h"
#include "data/lexer/lexer_error.h"
#include "data/lexer/lexer_result.h"
#include "data/lexer/token.h"
#include "data/source/location.h"
#include "data/source/number_literal.h"
#include "data/source/string_literal.h"
#include "data/util/text_utils.h"

namespace amelia {

namespace {

const Map<Text, TokenType> keywords = {
    {"fun", TokenType::KEYWORD_FUN},
    {"if", TokenType::KEYWORD_IF},
    {"else", TokenType::KEYWORD_ELSE},
    {"try", TokenType::KEYWORD_TRY},
    {"catch", TokenType::KEYWORD_CATCH},
    {"static", TokenType::KEYWORD_STATIC},
    {"this", TokenType::KEYWORD_THIS},
    {"This", TokenType::KEYWORD_THIS_TYPE},
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
    {"abstract", TokenType::KEYWORD_ABSTRACT},
    {"super", TokenType::KEYWORD_SUPER},
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

class LexerState {
public:
  LexerState(LexerResult &output, LexerContext ctx, Text file_contents) noexcept
      : m_ctx(ctx), m_line(1), m_column(1), m_file_contents(file_contents),
        m_input_iter(file_contents.begin()), m_previous_char_was_whitespace(true),
        m_result(output) {}

  void read_file() {
    while (!at_end()) {
      read_token();
    }

    emit_token(TokenType::END_OF_FILE, current_location());
  }

  void read_token() {
    uint32_t cp = peek();
    auto start_location = current_location();
    if (is_whitespace(cp)) {
      next();
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
    } else if (cp == '`') {
      read_quoted_ident(start_location);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(start_location, std::move(msg));
    }
  }

  void read_quoted_ident(Location start_location) {
    next();
    while (true) {
      if (at_end()) {
        throw_lexer_error_at_current_location(
            "Unterminated quoted identifier - unexpected end of input"
        );
      }
      uint32_t ch = peek();
      if (ch == '`') {
        next();
        if (!at_end() && peek() == '`') {
          next();
        } else {
          break;
        }
      } else {
        next();
      }
    }
    if (previous_char_was_whitespace()) {
      emit_token(TokenType::QUOTED_IDENTIFIER, start_location);
    } else {
      emit_token(TokenType::QUOTED_IDENTIFIER_NO_W, start_location);
    }
  }

  void read_quote(Location start_location) {
    Text first_three_chars = peek_n(3);
    if (first_three_chars == "\"\"\"") {
      read_string(start_location, true, false);
    } else {
      read_string(start_location, false, false);
    }
  }

  void read_raw_quote(Location start_location) {
    Text first_three_chars = peek_n(3);
    if (first_three_chars == "\"\"\"") {
      read_string(start_location, true, true);
    } else {
      read_string(start_location, false, true);
    }
  }

  void read_string(Location start_location, bool multiline, bool is_raw) {
    try {
      read_string_literal(start_location, multiline, is_raw);
    } catch (InvalidUTF8Error &e) {
      String err;
      err.append("Invalid string literal: ");
      err.append(Text::from(e.what()));
      throw_lexer_error(start_location, std::move(err));
    }
  }

  void read_string_literal(Location start_location, bool multiline, bool is_raw) {
    size_t quote_count = multiline ? 3 : 1;
    for (size_t i = 0; i < quote_count; ++i) {
      if (at_end() || next() != '"') {
        throw std::runtime_error("String literal does not start with expected quote characters");
      }
    }
    size_t literal_buffer_start_size = m_result.string_literal_buffer_size();
    size_t quotes_in_a_row = 0;
    while (true) {
      if (at_end()) {
        throw_lexer_error_at_current_location(
            "Unterminated string literal - unexpected end of input"
        );
      }
      uint32_t ch = peek();
      if (ch == '\\' && !is_raw) {
        next();
        if (at_end()) {
          throw_lexer_error_at_current_location(
              "Unexpected end of input after backslash in string literal"
          );
        }
        ch = peek();
        switch (ch) {
        case 'a':
          next();
          m_result.append_byte_to_string_literal_buffer('\a');
          break;
        case 'b':
          next();
          m_result.append_byte_to_string_literal_buffer('\b');
          break;
        case 'f':
          next();
          m_result.append_byte_to_string_literal_buffer('\f');
          break;
        case 'n':
          next();
          m_result.append_byte_to_string_literal_buffer('\n');
          break;
        case 'r':
          next();
          m_result.append_byte_to_string_literal_buffer('\r');
          break;
        case 't':
          next();
          m_result.append_byte_to_string_literal_buffer('\t');
          break;
        case 'v':
          next();
          m_result.append_byte_to_string_literal_buffer('\v');
          break;
        case '\\':
          next();
          m_result.append_byte_to_string_literal_buffer('\\');
          break;
        case '\'':
          next();
          m_result.append_byte_to_string_literal_buffer('\'');
          break;
        case '"':
          next();
          m_result.append_byte_to_string_literal_buffer('\"');
          break;
        case '\r':
          next();
          if (!at_end() && peek() == '\n') {
            next();
          }
          break;
        case '\n':
          next();
          break;
        case 'x':
          next();
          m_result.append_byte_to_string_literal_buffer(static_cast<char>(read_hex_chars(2)));
          break;
        case 'u':
          next();
          m_result.append_code_point_to_string_literal_buffer(read_hex_chars(4));
          break;
        case 'U':
          next();
          m_result.append_code_point_to_string_literal_buffer(read_hex_chars(8));
          break;
        default:
          String msg = "Invalid escape sequence: '\\";
          msg.append(ch);
          msg.append('\'');
          throw_lexer_error_at_current_location(std::move(msg));
        }
      } else if (ch == '\r') {
        // skip
        next();
      } else if (ch == '"') {
        next();
        ++quotes_in_a_row;
        if (quotes_in_a_row == quote_count) {
          break;
        }
      } else {
        if (ch == '\n' && !multiline) {
          throw_lexer_error_at_current_location("Unterminated string literal - unexpected newline");
        }
        while (quotes_in_a_row > 0) {
          m_result.append_byte_to_string_literal_buffer('"');
          --quotes_in_a_row;
        }
        m_result.append_code_point_to_string_literal_buffer(ch);
        next();
      }
    }
    size_t token_id = emit_token(TokenType::STRING_LITERAL, start_location);
    auto result = StringLiteral{literal_buffer_start_size, m_result.string_literal_buffer_size()};
    m_result.add_string_literal(token_id, result);
  }

  uint32_t read_hex_chars(size_t num_chars) {
    uint32_t result = 0;
    for (size_t i = 0; i < num_chars; ++i) {
      if (at_end()) {
        throw_lexer_error_at_current_location(
            "Unexpected end of input in hexadecimal escape sequence"
        );
      }
      uint32_t ch = next();
      result <<= 4;
      if (ch >= '0' && ch <= '9') {
        result |= (ch - '0');
      } else if (ch >= 'a' && ch <= 'f') {
        result |= (ch - 'a' + 10);
      } else if (ch >= 'A' && ch <= 'F') {
        result |= (ch - 'A' + 10);
      } else {
        throw_lexer_error_at_current_location("Invalid character in hexadecimal escape sequence");
      }
    }
    return result;
  }

  void read_at(Location start_location) {
    next();
    emit_token(TokenType::AT, start_location);
  }

  void read_right_bracket(Location start_location) {
    next();
    emit_token(TokenType::RIGHT_BRACKET, start_location);
  }

  void read_left_bracket(Location start_location) {
    next();
    if (previous_char_was_whitespace()) {
      emit_token(TokenType::LEFT_BRACKET, start_location);
    } else {
      emit_token(TokenType::LEFT_BRACKET_NO_W, start_location);
    }
  }

  void read_right_paren(Location start_location) {
    next();
    emit_token(TokenType::RIGHT_PAREN, start_location);
  }

  void read_left_paren(Location start_location) {
    next();
    if (previous_char_was_whitespace()) {
      emit_token(TokenType::LEFT_PAREN, start_location);
    } else {
      emit_token(TokenType::LEFT_PAREN_NO_W, start_location);
    }
  }

  void read_colon(Location start_location) {
    next();
    if (peek() == ':' && !previous_char_was_whitespace()) {
      next();
      emit_token(TokenType::DOUBLE_COLON_NO_W, start_location);
    } else {
      emit_token(TokenType::COLON, start_location);
    }
  }

  void read_dot(Location start_location) {
    next();
    if (TextUtils::is_digit(peek())) {
      set_location(start_location);
      read_number(start_location);
    } else if (peek() == '.') {
      next();
      if (peek() == '.') {
        next();
        emit_token(TokenType::ELLIPSIS, start_location);
      } else {
        throw_lexer_error(start_location, "Unexpected token '..' - did you mean '...'?");
      }
    } else if (previous_char_was_whitespace()) {
      emit_token(TokenType::DOT, start_location);
    } else {
      emit_token(TokenType::DOT_NO_W, start_location);
    }
  }

  void read_question_mark(Location start_location) {
    if (previous_char_was_whitespace()) {
      throw_lexer_error(start_location, "Unexpected '?' after whitespace");
    }
    next();
    emit_token(TokenType::QUESTION_NO_W, start_location);
  }

  void read_comma(Location start_location) {
    next();
    emit_token(TokenType::COMMA, start_location);
  }

  void read_semicolon(Location start_location) {
    next();
    emit_token(TokenType::SEMICOLON, start_location);
  }

  void read_left_brace(Location start_location) {
    next();
    emit_token(TokenType::LEFT_BRACE, start_location);
  }

  void read_right_brace(Location start_location) {
    next();
    emit_token(TokenType::RIGHT_BRACE, start_location);
  }

  void read_less(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::LESS_EQUAL, start_location);
    } else if (peek() == '<') {
      next();
      if (peek() == '=') {
        next();
        emit_token(TokenType::LSHIFT_EQUAL, start_location);
      } else {
        emit_token(TokenType::LSHIFT, start_location);
      }
    } else {
      emit_token(TokenType::LESS, start_location);
    }
  }

  void read_greater(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::GREATER_EQUAL, start_location);
    } else if (peek() == '>') {
      next();
      if (peek() == '=') {
        next();
        emit_token(TokenType::RSHIFT_EQUAL, start_location);
      } else {
        emit_token(TokenType::RSHIFT, start_location);
      }
    } else {
      emit_token(TokenType::GREATER, start_location);
    }
  }

  void read_tilde(Location start_location) {
    next();
    emit_token(TokenType::TILDE, start_location);
  }

  void read_exclamation(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::NOT_EQUAL, start_location);
    } else if (!previous_char_was_whitespace()) {
      emit_token(TokenType::EXCLAM_NO_W, start_location);
    } else {
      emit_token(TokenType::EXCLAM, start_location);
    }
  }

  void read_ampersand(Location start_location) {
    next();
    uint32_t next_cp = peek();
    if (next_cp == '&') {
      next();
      if (peek() == '=') {
        next();
        emit_token(TokenType::AND_EQUAL, start_location);
      } else {
        emit_token(TokenType::AND, start_location);
      }
    } else if (next_cp == '=') {
      next();
      emit_token(TokenType::AMPERSAND_EQUAL, start_location);
    } else {
      emit_token(TokenType::AMPERSAND, start_location);
    }
  }

  void read_pipe(Location start_location) {
    next();
    uint32_t next_cp = peek();
    if (next_cp == '=') {
      next();
      emit_token(TokenType::PIPE_EQUAL, start_location);
    } else if (next_cp == '|') {
      next();
      if (peek() == '=') {
        next();
        emit_token(TokenType::OR_EQUAL, start_location);
      } else {
        emit_token(TokenType::OR, start_location);
      }
    } else {
      emit_token(TokenType::PIPE, start_location);
    }
  }

  void read_caret(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::CARET_EQUAL, start_location);
    } else {
      emit_token(TokenType::CARET, start_location);
    }
  }

  void read_percent(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::PERCENT_EQUAL, start_location);
    } else {
      emit_token(TokenType::PERCENT, start_location);
    }
  }

  void read_star(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::STAR_EQUAL, start_location);
    } else {
      emit_token(TokenType::STAR, start_location);
    }
  }

  void read_minus(Location start_location) {
    next();
    uint32_t next_cp = peek();
    if (next_cp == '=') {
      next();
      emit_token(TokenType::MINUS_EQUAL, start_location);
    } else if (next_cp == '-') {
      next();
      if (previous_char_was_whitespace()) {
        emit_token(TokenType::DOUBLE_MINUS, start_location);
      } else {
        emit_token(TokenType::DOUBLE_MINUS_NO_W, start_location);
      }
    } else if (next_cp == '>') {
      next();
      emit_token(TokenType::ARROW, start_location);
    } else {
      emit_token(TokenType::MINUS, start_location);
    }
  }

  void read_plus(Location start_location) {
    next();
    auto next_cp = peek();
    if (next_cp == '=') {
      next();
      emit_token(TokenType::PLUS_EQUAL, start_location);
    } else if (next_cp == '+') {
      next();
      if (previous_char_was_whitespace()) {
        emit_token(TokenType::DOUBLE_PLUS, start_location);
      } else {
        emit_token(TokenType::DOUBLE_PLUS_NO_W, start_location);
      }
    } else {
      emit_token(TokenType::PLUS, start_location);
    }
  }

  void read_number(Location start_location) {
    NumberLiteral result;
    result.has_decimal_point = false;
    unsigned char base = 10;
    bool at_boundary = true;
    bool assumed_octal = false;
    bool previous_char_was_underscore = false;

    auto base_prefix_start = current_location();

    if (at_end()) {
      throw std::runtime_error("Expected number literal, but got empty input");
    }

    if (peek() == '0') {
      next();
      if (!at_end()) {
        auto ch = peek();
        if (ch == 'x' || ch == 'X') {
          base = 16;
          next();
        } else if (ch == 'b' || ch == 'B') {
          base = 2;
          next();
        } else if (ch == 'o' || ch == 'O') {
          base = 8;
          next();
        } else if (ch == '_' || TextUtils::is_digit(ch)) {
          base = 8;
          assumed_octal = true;
        }
      }

      if (base == 10) {
        // this zero was not the beginning of a base prefix. go back to parsing the number normally.
        set_location(base_prefix_start);
      } else {
        result.base_prefix = substr_after(base_prefix_start);
        at_boundary = false;
      }
    }

    auto integer_digits_start = current_location();
    while (!at_end()) {
      auto ch = peek();
      signed char digit_value = -1;
      if (ch == '_') {
        if (at_boundary || previous_char_was_underscore) {
          throw_lexer_error_at_current_location("Underscore must separate successive digits");
        }
        previous_char_was_underscore = true;
        next();
      } else if (TextUtils::is_digit(ch)) {
        digit_value = ch - '0';
      } else if (TextUtils::is_alpha(ch)) {
        if ((ch == 'e' || ch == 'E') && (base == 10 || assumed_octal)) {
          break;
        } else if (ch == 'p' || ch == 'P') {
          if (base == 16) {
            break;
          } else {
            throw_lexer_error(
                current_location(),
                "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
            );
          }
        } else if (ch >= 'a' && ch <= 'f') {
          digit_value = 10 + (ch - 'a');
        } else if (ch >= 'A' && ch <= 'F') {
          digit_value = 10 + (ch - 'A');
        } else {
          String err("Invalid character '");
          err.append(ch);
          err.append("' in number literal");
          throw_lexer_error_at_current_location(std::move(err));
        }
      }

      if (digit_value != -1) {
        if (digit_value >= base) {
          String err("Invalid digit '");
          err.append(ch);
          err.append("' for base ");
          TextUtils::to_string(err, int64_t(base));
          throw_lexer_error_at_current_location(std::move(err));
        }
        previous_char_was_underscore = false;
        next();
        at_boundary = false;
      } else if (ch != '_') {
        break;
      }
    }

    if (previous_char_was_underscore) {
      throw_lexer_error_at_current_location("Underscore must separate successive digits");
    }
    at_boundary = true;
    previous_char_was_underscore = false;
    result.integer_digits = substr_after(integer_digits_start);

    if (!at_end() && peek() == '.') {

      result.has_decimal_point = true;

      if (assumed_octal) {
        // a number with a leading zero is only assumed octal if it has no decimal point or exponent
        result.base_prefix = Text();
        result.integer_digits = substr_after(base_prefix_start);
        assumed_octal = false;
        base = 10;
      }

      if (base != 10 && base != 16) {
        throw_lexer_error_at_current_location("Floating point literals may only be in base 10 or 16"
        );
      }

      next();
      auto fractional_digits_start = current_location();
      while (!at_end()) {
        auto ch = peek();
        signed char digit_value = -1;
        if (ch == '_') {
          if (at_boundary || previous_char_was_underscore) {
            throw_lexer_error_at_current_location("Underscore must separate successive digits");
          }
          previous_char_was_underscore = true;
          next();
        } else if (TextUtils::is_digit(ch)) {
          digit_value = ch - '0';
        } else if (TextUtils::is_alpha(ch)) {
          if ((ch == 'e' || ch == 'E') && base == 10) {
            break;
          } else if ((ch == 'p' || ch == 'P') && base == 16) {
            break;
          } else if (ch == 'p' || ch == 'P') {
            throw_lexer_error(
                current_location(),
                "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
            );
          } else if (ch >= 'a' && ch <= 'f') {
            digit_value = 10 + (ch - 'a');
          } else if (ch >= 'A' && ch <= 'F') {
            digit_value = 10 + (ch - 'A');
          } else {
            String err("Invalid character '");
            err.append(ch);
            err.append("' in number literal");
            throw_lexer_error_at_current_location(std::move(err));
          }
        }

        if (digit_value != -1) {
          if (digit_value >= base) {
            String err("Invalid digit '");
            err.append(ch);
            err.append("' for base ");
            TextUtils::to_string(err, int64_t(base));
            throw_lexer_error_at_current_location(std::move(err));
          }
          previous_char_was_underscore = false;
          next();
          at_boundary = false;
        } else if (ch != '_') {
          break;
        }
      }

      result.fractional_digits = substr_after(fractional_digits_start);
    }

    if (previous_char_was_underscore) {
      throw_lexer_error_at_current_location("Underscore must separate successive digits");
    }
    at_boundary = true;
    previous_char_was_underscore = false;

    auto exponent_prefix_start = current_location();
    if (!at_end()) {
      auto ch = peek();
      if (ch == 'e' || ch == 'E') {
        if (base == 16) {
          throw_lexer_error_at_current_location(
              "Hexadecimal literals must use 'p' or 'P' as the exponent prefix"
          );
        }
        next();
      } else if (ch == 'p' || ch == 'P') {
        if (base != 16) {
          throw_lexer_error_at_current_location(
              "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
          );
        }
        next();
      }
    }
    result.exponent_prefix = substr_after(exponent_prefix_start);

    if (result.exponent_prefix.size() != 0) {
      if (at_end()) {
        throw_lexer_error_at_current_location("Exponent has no digits");
      }

      if (assumed_octal) {
        // a number with a leading zero is only assumed octal if it has no decimal point or exponent
        result.base_prefix = Text();
        result.integer_digits = substr(base_prefix_start, exponent_prefix_start);
        assumed_octal = false;
        base = 10;
      }

      if (base != 10 && base != 16) {
        throw_lexer_error_at_current_location("Only base 10 or 16 literals may have an exponent");
      }

      auto exponent_sign_start = current_location();
      if (peek() == '+' || peek() == '-') {
        next();
      }
      result.exponent_sign = substr_after(exponent_sign_start);

      if (at_end()) {
        throw_lexer_error_at_current_location("Exponent has no digits");
      }

      auto exponent_digits_start = current_location();
      while (!at_end()) {
        auto ch = peek();
        if (ch == '_') {
          if (at_boundary || previous_char_was_underscore) {
            throw_lexer_error_at_current_location("Underscore must separate successive digits");
          }
          previous_char_was_underscore = true;
          next();
        } else if (TextUtils::is_digit(ch)) {
          previous_char_was_underscore = false;
          next();
          at_boundary = false;
        } else if (ch == '.' || TextUtils::is_alpha(ch)) {
          String err("Invalid character '");
          err.append(ch);
          err.append("' in exponent");
          throw_lexer_error_at_current_location(std::move(err));
        } else if (ch != '_') {
          break;
        }
      }

      result.exponent_digits = substr_after(exponent_digits_start);
    }

    if (previous_char_was_underscore) {
      throw_lexer_error_at_current_location("Underscore must separate successive digits");
    }

    if (result.integer_digits.size() == 0 && result.fractional_digits.size() == 0) {
      throw_lexer_error_at_current_location("Number literal must have at least one digit");
    }

    size_t token_id;
    if (!previous_char_was_whitespace() && result.base_prefix.size() == 0 &&
        result.exponent_digits.size() == 0 && result.exponent_sign.size() == 0 &&
        result.exponent_prefix.size() == 0 && result.integer_digits.size() == 0 &&
        result.has_decimal_point && result.fractional_digits.size() > 0) {
      token_id = emit_token(TokenType::NUMBER_FIELD, start_location);
    } else {
      token_id = emit_token(TokenType::NUMBER, start_location);
    }
    m_result.add_number_literal(token_id, result);
  }

  void read_equal(Location start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::EQUAL, start_location);
    } else {
      emit_token(TokenType::ASSIGN, start_location);
    }
  }

  void read_slash(Location start_location) {
    next();
    uint32_t next_cp = peek();
    if (next_cp == '/') {
      next();
      skip_until_end_of_single_line_comment();
    } else if (next_cp == '*') {
      next();
      skip_until_end_of_multiline_comment();
    } else if (next_cp == '=') {
      next();
      emit_token(TokenType::SLASH_EQUAL, start_location);
    } else {
      emit_token(TokenType::SLASH, start_location);
    }
  }

  void skip_until_end_of_single_line_comment() {
    while (!at_end() && next() != '\n') {
      // skip
    }
  }

  void skip_until_end_of_multiline_comment() {
    uint32_t prev = 0;
    uint32_t cp = 0;
    while (!at_end()) {
      prev = cp;
      cp = next();
      if (prev == '*' && cp == '/') {
        break;
      }
    }
  }

  void read_word(Location start_location) {
    if (peek() == 'r') {
      next();
      if (peek() == '"') {
        read_raw_quote(start_location);
        return;
      }
    }
    skip_word_chars();

    Text word = substr_after(start_location);
    auto keyword_tt = keywords.find(word);

    if (keyword_tt.has_value()) {
      emit_token(*keyword_tt, start_location);
    } else if (previous_char_was_whitespace()) {
      emit_token(TokenType::IDENTIFIER, start_location);
    } else {
      emit_token(TokenType::IDENTIFIER_NO_W, start_location);
    }
  }

  void skip_word_chars() {
    while (!at_end() && is_word_continue(peek())) {
      next();
    }
  }

private:
  bool previous_char_was_whitespace() const {
    return m_previous_char_was_whitespace;
  }

  Text substr_after(Location start) const {
    return TextUtils::substr(m_file_contents, start.position, m_input_iter);
  }

  Text substr(Location start, Location end) const {
    return TextUtils::substr(m_file_contents, start.position, end.position);
  }

  Text peek_n(size_t n) const {
    return TextUtils::slice(m_file_contents, m_input_iter, n);
  }

  uint32_t next() {
    uint32_t cp = m_input_iter.next();
    if (is_whitespace(cp)) {
      m_previous_char_was_whitespace = true;
    }
    if (cp == '\n') {
      ++m_line;
      m_column = 1;
    } else {
      ++m_column;
    }
    return cp;
  }

  uint32_t peek() {
    return m_input_iter.peek();
  }

  bool at_end() const {
    return m_input_iter.at_end();
  }

  Location current_location() const noexcept {
    return Location{m_ctx.filename, m_input_iter, m_line, m_column};
  }

  void set_location(Location loc) {
    m_input_iter = loc.position;
    m_line = loc.line;
    m_column = loc.column;
  }

  size_t emit_token(TokenType type, Location loc) {
    return emit_token(type, loc, current_location());
  }

  size_t emit_token(TokenType type, Location start, Location end) {
    size_t token_id = m_result.add_token(
        Token{type, start, TextUtils::substr(m_file_contents, start.position, end.position)}
    );
    m_previous_char_was_whitespace = false;
    return token_id;
  }

  [[noreturn]] void throw_lexer_error_at_current_location(String message) {
    throw_lexer_error(current_location(), std::move(message));
  }

  [[noreturn]] void throw_lexer_error(Location loc, String message) {
    throw LexerError(loc, std::move(message));
  }

  LexerContext m_ctx;
  size_t m_line;
  size_t m_column;
  Text m_file_contents;
  CharIterator m_input_iter;
  bool m_previous_char_was_whitespace;

  LexerResult &m_result;
};

} // namespace

void Lexer::tokenize(LexerResult &output, LexerContext ctx, Text input) {
  LexerState state(output, ctx, input);
  state.read_file();
}

} // namespace amelia
