#include "lexer.hpp"

#include "lexer/data/lexer_context.hpp"
#include "lexer/data/lexer_error.hpp"
#include "lexer/data/lexer_result.hpp"
#include "lexer/data/token.hpp"
#include "source/data/identifier.hpp"
#include "source/data/location.hpp"
#include "source/data/number_literal.hpp"
#include "util/data/map.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

const Map<Text, TokenType> RESERVED_WORDS = Map<Text, TokenType>({
    {"fun", TokenType::KEYWORD_FUN},
    {"if", TokenType::KEYWORD_IF},
    {"else", TokenType::KEYWORD_ELSE},
    {"try", TokenType::KEYWORD_TRY},
    {"catch", TokenType::KEYWORD_CATCH},
    {"static", TokenType::KEYWORD_STATIC},
    {"this", TokenType::KEYWORD_THIS},
    {"This", TokenType::KEYWORD_THIS_TYPE},
    {"module", TokenType::KEYWORD_MODULE},
    {"bool", TokenType::KEYWORD_BOOL},
    {"byte", TokenType::KEYWORD_BYTE},
    {"short", TokenType::KEYWORD_SHORT},
    {"int", TokenType::KEYWORD_INT},
    {"long", TokenType::KEYWORD_LONG},
    {"ubyte", TokenType::KEYWORD_UBYTE},
    {"ushort", TokenType::KEYWORD_USHORT},
    {"uint", TokenType::KEYWORD_UINT},
    {"ulong", TokenType::KEYWORD_ULONG},
    {"usize", TokenType::KEYWORD_USIZE},
    {"float", TokenType::KEYWORD_FLOAT},
    {"double", TokenType::KEYWORD_DOUBLE},
    {"bitint", TokenType::KEYWORD_BITINT},
    {"ubitint", TokenType::KEYWORD_UBITINT},
    {"char", TokenType::KEYWORD_CHAR},
    {"text", TokenType::KEYWORD_TEXT},
    {"null", TokenType::KEYWORD_NULL},
    {"never", TokenType::KEYWORD_NEVER},
    {"throw", TokenType::KEYWORD_THROW},
    {"import", TokenType::KEYWORD_IMPORT},
    {"as", TokenType::KEYWORD_AS},
    {"is", TokenType::KEYWORD_IS},
    {"class", TokenType::KEYWORD_CLASS},
    {"union", TokenType::KEYWORD_UNION},
    {"record", TokenType::KEYWORD_RECORD},
    {"sealed", TokenType::KEYWORD_SEALED},
    {"type", TokenType::KEYWORD_TYPE},
    {"typeof", TokenType::KEYWORD_TYPEOF},
    {"concept", TokenType::KEYWORD_CONCEPT},
    {"let", TokenType::KEYWORD_LET},
    {"const", TokenType::KEYWORD_CONST},
    {"impl", TokenType::KEYWORD_IMPL},
    {"any", TokenType::KEYWORD_ANY},
    {"goto", TokenType::KEYWORD_GOTO},
    {"async", TokenType::KEYWORD_ASYNC},
    {"await", TokenType::KEYWORD_AWAIT},
    {"true", TokenType::KEYWORD_TRUE},
    {"false", TokenType::KEYWORD_FALSE},
    {"default", TokenType::KEYWORD_DEFAULT},
    {"open", TokenType::KEYWORD_OPEN},
    {"override", TokenType::KEYWORD_OVERRIDE},
    {"mut", TokenType::KEYWORD_MUT},
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
    {"super", TokenType::KEYWORD_SUPER},
    {"abstract", TokenType::KEYWORD_ABSTRACT},
    {"_", TokenType::KEYWORD_UNDERSCORE},
});

namespace {
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
        m_input_iter(file_contents.begin()),
        m_previous_char_was_whitespace(true), m_result(output) {
    skip_carriage_returns();
  }

  void read_file() {
    while (!at_end()) {
      read_token();
    }

    emit_token(TokenType::END_OF_FILE, current_location());
  }

  void read_string_literal_contents(AbstractString &output, bool raw) {
    auto start_location = current_location();
    List<char> x_scratch_buffer;
    List<uint16_t> u_scratch_buffer;
    int64_t current_char = -1, next_char = -1;
    while (true) {
      if (!at_end()) {
        current_char = next();
        if (!at_end()) {
          next_char = peek();
        } else {
          next_char = -1;
        }
      } else {
        current_char = -1;
      }

      if (x_scratch_buffer.size() != 0 &&
          !(current_char == '\\' && next_char == 'x')) {
        try {
          output.append(Text(x_scratch_buffer.data()));
        } catch (const RuntimeError &err) {
          String error_message("Content error: ");
          error_message.append(Text::from(err.what()));
          throw_lexer_error(start_location, move(error_message));
        }
        x_scratch_buffer.clear();
      }

      if (u_scratch_buffer.size() != 0 &&
          !(current_char == '\\' && next_char == 'u')) {
        try {
          CharIterator::utf16_to_utf8(output, u_scratch_buffer.data());
        } catch (const RuntimeError &err) {
          String error_message("Content error: ");
          error_message.append(Text::from(err.what()));
          throw_lexer_error(start_location, move(error_message));
        }
        u_scratch_buffer.clear();
      }

      if (current_char == -1) {
        return;
      }

      if (!raw && current_char == '\\') {
        if (next_char == -1) {
          throw_lexer_error_at_current_location(
              "Unexpected end of string after backslash");
        }

        if (next_char == 'x') {
          next(); // skip past the 'x'
          if (at_end()) {
            throw_lexer_error_at_current_location(
                "Unexpected end of input in hexadecimal escape sequence");
          }
          if (peek() == '{') {
            x_scratch_buffer.push_back(
                static_cast<char>(read_hex_chars_in_braces(2)));
          } else {
            x_scratch_buffer.push_back(static_cast<char>(read_hex_chars(2)));
          }
          continue;
        }

        if (next_char == 'u') {
          next(); // skip past the 'u'
          if (at_end()) {
            throw_lexer_error_at_current_location(
                "Unexpected end of input in hexadecimal escape sequence");
          }
          if (peek() == '{') {
            u_scratch_buffer.push_back(
                static_cast<uint16_t>(read_hex_chars_in_braces(4)));
          } else {
            u_scratch_buffer.push_back(
                static_cast<uint16_t>(read_hex_chars(4)));
          }
          continue;
        }

        if (next_char == 'U') {
          next(); // skip past the 'U'
          if (at_end()) {
            throw_lexer_error_at_current_location(
                "Unexpected end of input in hexadecimal escape sequence");
          }
          uint32_t character;
          if (peek() == '{') {
            character = read_hex_chars_in_braces(8);
          } else {
            character = read_hex_chars(8);
          }
          try {
            output.append(character);
          } catch (const RuntimeError &err) {
            String error_message("Content error: ");
            error_message.append(Text::from(err.what()));
            throw_lexer_error(start_location, move(error_message));
          }
          continue;
        }

        uint32_t result;
        if (next_char >= '0' && next_char <= '7') {
          result = read_octal_chars();
        } else {
          next(); // skip past escaped character
          switch (next_char) {
          case -1:
            throw_lexer_error_at_current_location(
                "Unexpected end of input after backslash");
          case 'a':
            result = '\a';
            break;
          case 'b':
            result = '\b';
            break;
          case 'f':
            result = '\f';
            break;
          case 'n':
            result = '\n';
            break;
          case 'r':
            result = '\r';
            break;
          case 't':
            result = '\t';
            break;
          case 'v':
            result = '\v';
            break;
          case '\n':
            continue;
          default:
            result = next_char;
          }
        }
        output.append(result);
        continue;
      }

      output.append(current_char);
    }
  }

  void read_number_literal_contents(NumberLiteral &result) {
    auto start_location = current_location();
    result.has_decimal_point = false;
    uint8_t base = 10;
    bool at_boundary = true;
    bool assumed_octal = false;
    bool previous_char_was_underscore = false;

    auto base_prefix_start = current_location();

    if (at_end()) {
      throw_lexer_error(start_location,
                        "Expected number literal, but got empty input");
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
        // this zero was not the beginning of a base prefix. go back to parsing
        // the number normally.
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
          throw_lexer_error_at_current_location(
              "Single underscore can only appear between digits");
        }
        previous_char_was_underscore = true;
        next();
      } else if (TextUtils::is_digit(ch)) {
        digit_value = static_cast<signed char>(ch - '0');
      } else if (TextUtils::is_alpha(ch)) {
        if ((ch == 'e' || ch == 'E') && (base == 10 || assumed_octal)) {
          break;
        } else if (ch == 'p' || ch == 'P') {
          break;
        } else if (ch >= 'a' && ch <= 'f') {
          digit_value = static_cast<signed char>(10 + (ch - 'a'));
        } else if (ch >= 'A' && ch <= 'F') {
          digit_value = static_cast<signed char>(10 + (ch - 'A'));
        } else {
          String err("Invalid character '");
          err.append(ch);
          err.append("' in number literal");
          throw_lexer_error_at_current_location(move(err));
        }
      }

      if (digit_value != -1) {
        if (digit_value >= base) {
          String err("Invalid digit '");
          err.append(ch);
          err.append("' for base ");
          TextUtils::to_string(err, static_cast<int64_t>(base));
          throw_lexer_error_at_current_location(move(err));
        }
        previous_char_was_underscore = false;
        next();
        at_boundary = false;
      } else if (ch != '_') {
        break;
      }
    }

    if (previous_char_was_underscore) {
      throw_lexer_error_at_current_location(
          "Single underscore can only appear between digits");
    }
    at_boundary = true;
    previous_char_was_underscore = false;
    result.integer_digits = substr_after(integer_digits_start);

    if (!at_end() && peek() == '.') {

      result.has_decimal_point = true;

      if (assumed_octal) {
        // a number with a leading zero is only assumed octal if it has no
        // decimal point or exponent
        result.base_prefix = Text();
        result.integer_digits = substr_after(base_prefix_start);
        assumed_octal = false;
        base = 10;
      }

      next();
      auto fractional_digits_start = current_location();
      while (!at_end()) {
        auto ch = peek();
        signed char digit_value = -1;
        if (ch == '_') {
          if (at_boundary || previous_char_was_underscore) {
            throw_lexer_error_at_current_location(
                "Single underscore can only appear between digits");
          }
          previous_char_was_underscore = true;
          next();
        } else if (TextUtils::is_digit(ch)) {
          digit_value = static_cast<signed char>(ch - '0');
        } else if (TextUtils::is_alpha(ch)) {
          if ((ch == 'e' || ch == 'E') && base == 10) {
            break;
          } else if (ch == 'p' || ch == 'P') {
            break;
          } else if (ch >= 'a' && ch <= 'f') {
            digit_value = static_cast<signed char>(10 + (ch - 'a'));
          } else if (ch >= 'A' && ch <= 'F') {
            digit_value = static_cast<signed char>(10 + (ch - 'A'));
          } else {
            String err("Invalid character '");
            err.append(ch);
            err.append("' in number literal");
            throw_lexer_error_at_current_location(move(err));
          }
        }

        if (digit_value != -1) {
          if (digit_value >= base) {
            String err("Invalid digit '");
            err.append(ch);
            err.append("' for base ");
            TextUtils::to_string(err, static_cast<int64_t>(base));
            throw_lexer_error_at_current_location(move(err));
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
      throw_lexer_error_at_current_location(
          "Single underscore can only appear between digits");
    }
    at_boundary = true;
    previous_char_was_underscore = false;

    auto exponent_prefix_start = current_location();
    if (!at_end()) {
      auto ch = peek();
      if (ch == 'e' || ch == 'E') {
        if (base == 16) {
          throw_lexer_error_at_current_location(
              "Hexadecimal literals must use 'p' or 'P' as the exponent "
              "prefix");
        }
        next();
      } else if (ch == 'p' || ch == 'P') {
        if (base != 16) {
          throw_lexer_error_at_current_location(
              "Only hexadecimal literals may use 'p' or 'P' as the exponent "
              "prefix");
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
        // a number with a leading zero is only assumed octal if it has no
        // decimal point or exponent
        result.base_prefix = Text();
        result.integer_digits =
            substr(base_prefix_start, exponent_prefix_start);
        assumed_octal = false;
        base = 10;
      }

      if (base != 10 && base != 16) {
        throw_lexer_error_at_current_location(
            "Only base 10 or 16 literals may have an exponent");
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
            throw_lexer_error_at_current_location(
                "Single underscore can only appear between digits");
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
          throw_lexer_error_at_current_location(move(err));
        } else if (ch != '_') {
          break;
        }
      }

      result.exponent_digits = substr_after(exponent_digits_start);
    }

    if (previous_char_was_underscore) {
      throw_lexer_error_at_current_location(
          "Single underscore can only appear between digits");
    }

    if (result.integer_digits.size() == 0 &&
        result.fractional_digits.size() == 0) {
      throw_lexer_error_at_current_location(
          "Number literal must have at least one digit");
    }
  }

  uint32_t read_char_literal_contents() {
    auto start_location = current_location();
    String result;
    read_string_literal_contents(result, false);
    size_t i = 0;
    uint32_t result_ch = 0;
    for (auto c : result) {
      ++i;
      result_ch = c;
    }
    if (i == 0) {
      throw_lexer_error(start_location, "Empty character literal");
    } else if (i > 1) {
      throw_lexer_error(start_location,
                        "Character literal contains more than one code point");
    }

    return result_ch;
  }

private:
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
    } else if (cp == '$') {
      read_dollar(start_location);
    } else if (cp == '"') {
      read_quote(start_location, false);
    } else if (cp == '\'') {
      read_char(start_location);
    } else if (TextUtils::is_digit(cp)) {
      read_number(start_location);
    } else if (is_word_start(cp)) {
      read_word(start_location);
    } else if (cp == '`') {
      read_quoted_ident(start_location, false);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(start_location, move(msg));
    }
  }

  void read_at(const Location &start_location) {
    next();
    emit_token(TokenType::AT, start_location);
  }

  void read_dollar(const Location &start_location) {
    next();
    emit_token(TokenType::DOLLAR, start_location);
  }

  void read_right_bracket(const Location &start_location) {
    next();
    emit_token(TokenType::RIGHT_BRACKET, start_location);
  }

  void read_left_bracket(const Location &start_location) {
    next();
    if (previous_char_was_whitespace()) {
      emit_token(TokenType::LEFT_BRACKET, start_location);
    } else {
      emit_token(TokenType::LEFT_BRACKET_NO_W, start_location);
    }
  }

  void read_right_paren(const Location &start_location) {
    next();
    emit_token(TokenType::RIGHT_PAREN, start_location);
  }

  void read_left_paren(const Location &start_location) {
    next();
    if (previous_char_was_whitespace()) {
      emit_token(TokenType::LEFT_PAREN, start_location);
    } else {
      emit_token(TokenType::LEFT_PAREN_NO_W, start_location);
    }
  }

  void read_colon(const Location &start_location) {
    next();
    if (peek() == ':' && !previous_char_was_whitespace()) {
      next();
      emit_token(TokenType::DOUBLE_COLON_NO_W, start_location);
    } else {
      emit_token(TokenType::COLON, start_location);
    }
  }

  void read_dot(const Location &start_location) {
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
        throw_lexer_error(start_location,
                          "Unexpected token '..' - did you mean '...'?");
      }
    } else {
      emit_token(TokenType::DOT, start_location);
    }
  }

  void read_question_mark(const Location &start_location) {
    if (previous_char_was_whitespace()) {
      throw_lexer_error(start_location, "Unexpected '?' after whitespace");
    }
    next();
    emit_token(TokenType::QUESTION_NO_W, start_location);
  }

  void read_comma(const Location &start_location) {
    next();
    emit_token(TokenType::COMMA, start_location);
  }

  void read_semicolon(const Location &start_location) {
    next();
    emit_token(TokenType::SEMICOLON, start_location);
  }

  void read_left_brace(const Location &start_location) {
    next();
    emit_token(TokenType::LEFT_BRACE, start_location);
  }

  void read_right_brace(const Location &start_location) {
    next();
    emit_token(TokenType::RIGHT_BRACE, start_location);
  }

  void read_less(const Location &start_location) {
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

  void read_greater(const Location &start_location) {
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

  void read_tilde(const Location &start_location) {
    next();
    emit_token(TokenType::TILDE, start_location);
  }

  void read_exclamation(const Location &start_location) {
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

  void read_ampersand(const Location &start_location) {
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

  void read_pipe(const Location &start_location) {
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

  void read_caret(const Location &start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::CARET_EQUAL, start_location);
    } else {
      emit_token(TokenType::CARET, start_location);
    }
  }

  void read_percent(const Location &start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::PERCENT_EQUAL, start_location);
    } else {
      emit_token(TokenType::PERCENT, start_location);
    }
  }

  void read_star(const Location &start_location) {
    next();
    if (peek() == '=') {
      next();
      emit_token(TokenType::STAR_EQUAL, start_location);
    } else {
      emit_token(TokenType::STAR, start_location);
    }
  }

  void read_minus(const Location &start_location) {
    next();
    if (uint32_t next_cp = peek(); next_cp == '=') {
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

  void read_plus(const Location &start_location) {
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

  void read_equal(const Location &start_location) {
    next();
    auto next_cp = peek();
    if (next_cp == '=') {
      next();
      emit_token(TokenType::EQUAL, start_location);
    } else if (next_cp == '>') {
      next();
      emit_token(TokenType::FAT_ARROW, start_location);
    } else {
      emit_token(TokenType::ASSIGN, start_location);
    }
  }

  void read_slash(const Location &start_location) {
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

  void read_word(const Location &start_location) {
    if (peek() == 'r') {
      next();
      if (peek() == '"') {
        read_quote(start_location, true);
        return;
      }
    }
    skip_word_chars();

    Text word = substr_after(start_location);
    auto keyword_tt = RESERVED_WORDS.find(word);

    if (keyword_tt.has_value()) {
      emit_token(*keyword_tt, start_location);
    } else if (previous_char_was_whitespace()) {
      emit_token(TokenType::IDENTIFIER, start_location);
    } else {
      emit_token(TokenType::IDENTIFIER_NO_W, start_location);
    }
  }

  void read_char(const Location &start_location) {
    next(); // skip opening quote
    if (at_end()) {
      throw_lexer_error(start_location, "Unterminated character literal");
    }
    auto content_start = current_location();
    auto ch = peek();
    while (ch != '\'') {
      if (ch == '\\') {
        next();
        if (at_end()) {
          throw_lexer_error(
              start_location,
              "Unexpected end of input after backslash in character literal");
        }
      } else if (ch == '\n') {
        throw_lexer_error(start_location, "Unterminated character literal");
      }
      next();
      if (at_end()) {
        throw_lexer_error(start_location, "Unterminated character literal");
      }
      ch = peek();
    }
    auto content_end = current_location();
    next(); // skip closing quote

    // validate contents
    make_sublexer(content_start, content_end).read_char_literal_contents();

    emit_token(TokenType::CHAR_LITERAL, start_location);
  }

  void read_number(const Location &start_location) {
    NumberLiteral result;
    read_number_literal_contents(result);
    if (!previous_char_was_whitespace() && result.base_prefix.size() == 0 &&
        result.exponent_digits.size() == 0 &&
        result.exponent_sign.size() == 0 &&
        result.exponent_prefix.size() == 0 &&
        result.integer_digits.size() == 0 && result.has_decimal_point &&
        result.fractional_digits.size() > 0) {
      emit_token(TokenType::NUMBER_FIELD, start_location);
    } else {
      emit_token(TokenType::NUMBER, start_location);
    }
  }

  void read_quoted_ident(const Location &start_location, bool raw) {
    next(); // skip '`'
    if (at_end()) {
      throw_lexer_error(start_location, "Unterminated quoted identifier");
    }
    auto content_start = current_location();
    auto ch = peek();
    while (ch != '`') {
      if (ch == '\n') {
        throw_lexer_error(start_location, "Unterminated quoted identifier");
      } else if (!raw && ch == '\\') {
        next();
        if (at_end()) {
          throw_lexer_error_at_current_location(
              "Unexpected end of input after backslash");
        }
      }

      next();
      if (at_end()) {
        throw_lexer_error(start_location, "Unterminated quoted identifier");
      }
      ch = peek();
    }
    auto content_end = current_location();
    next(); // skip closing '`'

    if (raw) {
      emit_token(previous_char_was_whitespace()
                     ? TokenType::RAW_QUOTED_IDENTIFIER
                     : TokenType::RAW_QUOTED_IDENTIFIER_NO_W,
                 start_location);
    } else {
      // validate literal contents
      String buffer;
      make_sublexer(content_start, content_end)
          .read_string_literal_contents(buffer, false);

      emit_token(previous_char_was_whitespace()
                     ? TokenType::QUOTED_IDENTIFIER
                     : TokenType::QUOTED_IDENTIFIER_NO_W,
                 start_location);
    }
  }

  void read_quote(const Location &start_location, bool raw) {
    size_t quote_count = 0;
    while (!at_end() && peek() == '"') {
      next();
      ++quote_count;
    }

    if (quote_count == 2) {
      // empty string literal
      emit_token(TokenType::STRING_LITERAL, start_location);
      return;
    }

    if (quote_count == 6) {
      // empty multiline string literal
      emit_token(TokenType::MULTILINE_STRING_LITERAL, start_location);
      return;
    }

    if (quote_count != 1 && quote_count != 3) {
      throw_lexer_error_at_current_location(
          "String literal must begin with 1 or 3 quotes");
    }

    size_t quotes_in_a_row = 0;
    auto content_start = current_location(), content_end = current_location();
    while (quotes_in_a_row < quote_count) {
      if (at_end()) {
        throw_lexer_error(start_location, "Unterminated string literal");
      }
      auto ch = next();
      if (ch == '"') {
        ++quotes_in_a_row;
      } else {
        quotes_in_a_row = 0;
        if (ch == '\n' && quote_count == 1) {
          throw_lexer_error(start_location, "Unterminated string literal");
        } else if (!raw && ch == '\\') {
          if (at_end()) {
            throw_lexer_error_at_current_location(
                "Unexpected end of input after backslash");
          }
          next(); // skip over escaped character
        }
        content_end = current_location();
      }
    }

    if (raw) {
      emit_token(quote_count == 1 ? TokenType::RAW_STRING_LITERAL
                                  : TokenType::RAW_MULTILINE_STRING_LITERAL,
                 start_location);
    } else {
      // validate string literal contents
      String buffer;
      make_sublexer(content_start, content_end)
          .read_string_literal_contents(buffer, false);

      emit_token(quote_count == 1 ? TokenType::STRING_LITERAL
                                  : TokenType::MULTILINE_STRING_LITERAL,
                 start_location);
    }
  }

  uint32_t read_octal_chars() {
    uint32_t result = 0;
    for (size_t i = 0; i < 3; ++i) {
      if (at_end()) {
        return result;
      }
      auto ch = next();
      if (!(ch >= '0' && ch <= '7')) {
        break;
      }
      result <<= 3;
      result |= (ch - '0');
    }
    return result;
  }

  uint32_t read_hex_chars(size_t count) {
    uint32_t result = 0;
    for (size_t i = 0; i < count; ++i) {
      if (at_end()) {
        throw_lexer_error_at_current_location(
            "Hexadecimal escape sequence too short");
      }
      auto ch = next();
      result <<= 4;
      if (ch >= '0' && ch <= '9') {
        result |= (ch - '0');
      } else if (ch >= 'a' && ch <= 'f') {
        result |= (ch - 'a' + 10);
      } else if (ch >= 'A' && ch <= 'F') {
        result |= (ch - 'A' + 10);
      } else {
        throw_lexer_error_at_current_location(
            "Invalid character in hexadecimal escape sequence");
      }
    }
    return result;
  }

  uint32_t read_hex_chars_in_braces(size_t max_count) {
    next(); // skip opening brace
    size_t count = 0, nonzero_count = 0;
    uint32_t result = 0;
    if (at_end()) {
      throw_lexer_error_at_current_location(
          "Unterminated hexadecimal escape sequence");
    }
    auto ch = next();
    while (ch != '}') {
      if (nonzero_count == max_count) {
        throw_lexer_error_at_current_location(
            "Too many characters in hexadecimal escape sequence");
      }
      ++count;
      if (!(nonzero_count == 0 && ch == '0')) { // ignore leading zeros
        ++nonzero_count;
        result <<= 4;
        if (ch >= '0' && ch <= '9') {
          result |= (ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
          result |= (ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
          result |= (ch - 'A' + 10);
        } else {
          throw_lexer_error_at_current_location(
              "Invalid character in hexadecimal escape sequence");
        }
      }
      if (at_end()) {
        throw_lexer_error_at_current_location(
            "Unterminated hexadecimal escape sequence");
      }
      ch = next();
    }
    if (count == 0) {
      throw_lexer_error_at_current_location(
          "Empty hexadecimal escape sequence");
    }
    return result;
  }

  void skip_word_chars() {
    while (!at_end() && is_word_continue(peek())) {
      next();
    }
  }

  bool previous_char_was_whitespace() const {
    return m_previous_char_was_whitespace;
  }

  Text substr_after(const Location &start) const {
    return TextUtils::substr(m_file_contents, start.position, m_input_iter);
  }

  Text substr(const Location &start, const Location &end) const {
    return TextUtils::substr(m_file_contents, start.position, end.position);
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
    skip_carriage_returns();
    return cp;
  }

  uint32_t peek() const { return m_input_iter.peek(); }

  bool at_end() const { return m_input_iter.at_end(); }

  Location current_location() const noexcept {
    return Location{m_ctx.filename, m_input_iter, m_line, m_column};
  }

  void set_location(const Location &loc) {
    m_input_iter = loc.position;
    m_line = loc.line;
    m_column = loc.column;
  }

  size_t emit_token(TokenType type, const Location &loc) {
    return emit_token(type, loc, current_location());
  }

  size_t emit_token(TokenType type, const Location &start,
                    const Location &end) {
    size_t token_id = m_result.add_token(Token{
        type, start,
        TextUtils::substr(m_file_contents, start.position, end.position)});
    m_previous_char_was_whitespace = false;
    return token_id;
  }

  void skip_carriage_returns() {
    while (!m_input_iter.at_end() && m_input_iter.peek() == '\r') {
      m_input_iter.next();
    }
  }

  LexerState make_sublexer(const Location &start, const Location &end) const {
    LexerState result = *this;
    result.m_previous_char_was_whitespace = true;
    result.m_line = start.line;
    result.m_column = start.column;
    result.m_input_iter =
        TextUtils::substr(m_file_contents, start.position, end.position)
            .begin();
    return result;
  }

  [[noreturn]] void
  throw_lexer_error_at_current_location(String message) const {
    throw_lexer_error(current_location(), move(message));
  }

  [[noreturn]] static void throw_lexer_error(const Location &loc,
                                             String message) {
    throw LexerError(loc, move(message));
  }

  LexerContext m_ctx;
  uint32_t m_line;
  uint32_t m_column;
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

void Lexer::read_string_literal_contents(AbstractString &out, Text input,
                                         bool raw) {
  LexerResult result;
  LexerState state(result, LexerContext{"(anon)"}, input);
  state.read_string_literal_contents(out, raw);
}

uint32_t Lexer::read_char_literal_contents(Text input) {
  String out;
  LexerResult result;
  LexerState state(result, LexerContext{"(anon)"}, input);
  return state.read_char_literal_contents();
}

void Lexer::read_number_literal_contents(NumberLiteral &result, Text input) {
  LexerResult lr;
  LexerState state(lr, LexerContext{"(anon)"}, input);
  state.read_number_literal_contents(result);
}

} // namespace amelia
