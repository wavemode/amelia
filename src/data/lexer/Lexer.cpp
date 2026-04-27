#include <cstdint>
#include <unordered_set>

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

const std::unordered_set<Text> keywords = {
    "fun",    "if",       "else",     "try",    "catch",   "static",    "this",     "module",
    "void",   "throw",    "import",   "as",     "switch",  "case",      "class",    "union",
    "record", "type",     "concept",  "bool",   "auto",    "let",       "const",    "impl",
    "any",    "goto",     "async",    "await",  "true",    "false",     "null",     "default",
    "open",   "override", "local",    "public", "private", "protected", "enum",     "copy",
    "move",   "operator", "extern",   "inline", "delete",  "new",       "implicit", "with",
    "when",   "return",   "continue", "break",  "while",   "for",       "in",
};

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
    } else if (TextUtils::is_digit(cp)) {
      read_number(start_location);
    } else if (is_ident_start(cp)) {
      read_identifier(start_location);
    } else {
      String msg = "Unexpected character: '";
      msg.append(cp);
      msg.append('\'');
      throw_lexer_error(std::move(msg));
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
    } else {
      throw_lexer_error("not implemented");
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

  void read_identifier(Location start_location) {
    while (!input.at_end() && is_ident_continue(input.peek())) {
      advance();
    }
    Text ident = TextUtils::substr(file_contents, start_location.position, input);
    if (keywords.find(ident) == keywords.end()) {
      emit_token(TokenType::IDENTIFIER, start_location);
    } else {
      emit_token(TokenType::KEYWORD, start_location);
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
