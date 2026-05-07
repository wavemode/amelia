#pragma once

#define TOKEN_TYPE_LIST                                                                            \
  X(KEYWORD_FUN)                                                                                   \
  X(KEYWORD_IF)                                                                                    \
  X(KEYWORD_ELSE)                                                                                  \
  X(KEYWORD_TRY)                                                                                   \
  X(KEYWORD_CATCH)                                                                                 \
  X(KEYWORD_STATIC)                                                                                \
  X(KEYWORD_THIS)                                                                                  \
  X(KEYWORD_MODULE)                                                                                \
  X(KEYWORD_VOID)                                                                                  \
  X(KEYWORD_THROW)                                                                                 \
  X(KEYWORD_IMPORT)                                                                                \
  X(KEYWORD_AS)                                                                                    \
  X(KEYWORD_SWITCH)                                                                                \
  X(KEYWORD_CASE)                                                                                  \
  X(KEYWORD_CLASS)                                                                                 \
  X(KEYWORD_UNION)                                                                                 \
  X(KEYWORD_RECORD)                                                                                \
  X(KEYWORD_TYPE)                                                                                  \
  X(KEYWORD_CONCEPT)                                                                               \
  X(KEYWORD_BOOL)                                                                                  \
  X(KEYWORD_AUTO)                                                                                  \
  X(KEYWORD_LET)                                                                                   \
  X(KEYWORD_CONST)                                                                                 \
  X(KEYWORD_IMPL)                                                                                  \
  X(KEYWORD_ANY)                                                                                   \
  X(KEYWORD_GOTO)                                                                                  \
  X(KEYWORD_ASYNC)                                                                                 \
  X(KEYWORD_AWAIT)                                                                                 \
  X(KEYWORD_TRUE)                                                                                  \
  X(KEYWORD_FALSE)                                                                                 \
  X(KEYWORD_NULL)                                                                                  \
  X(KEYWORD_DEFAULT)                                                                               \
  X(KEYWORD_OPEN)                                                                                  \
  X(KEYWORD_OVERRIDE)                                                                              \
  X(KEYWORD_LOCAL)                                                                                 \
  X(KEYWORD_PUBLIC)                                                                                \
  X(KEYWORD_PRIVATE)                                                                               \
  X(KEYWORD_PROTECTED)                                                                             \
  X(KEYWORD_ENUM)                                                                                  \
  X(KEYWORD_COPY)                                                                                  \
  X(KEYWORD_MOVE)                                                                                  \
  X(KEYWORD_OPERATOR)                                                                              \
  X(KEYWORD_EXTERN)                                                                                \
  X(KEYWORD_INLINE)                                                                                \
  X(KEYWORD_DELETE)                                                                                \
  X(KEYWORD_NEW)                                                                                   \
  X(KEYWORD_IMPLICIT)                                                                              \
  X(KEYWORD_WITH)                                                                                  \
  X(KEYWORD_WHEN)                                                                                  \
  X(KEYWORD_RETURN)                                                                                \
  X(KEYWORD_CONTINUE)                                                                              \
  X(KEYWORD_BREAK)                                                                                 \
  X(KEYWORD_WHILE)                                                                                 \
  X(KEYWORD_FOR)                                                                                   \
  X(KEYWORD_IN)                                                                                    \
  X(KEYWORD_LABEL)                                                                                 \
  X(KEYWORD_ABSTRACT)                                                                              \
  X(KEYWORD_SUPER)                                                                                 \
  X(ELLIPSIS)                                                                                      \
  X(ASSIGN)                                                                                        \
  X(PLUS_EQUAL)                                                                                    \
  X(MINUS_EQUAL)                                                                                   \
  X(STAR_EQUAL)                                                                                    \
  X(SLASH_EQUAL)                                                                                   \
  X(PERCENT)                                                                                       \
  X(PERCENT_EQUAL)                                                                                 \
  X(PIPE)                                                                                          \
  X(PIPE_EQUAL)                                                                                    \
  X(AMPERSAND)                                                                                     \
  X(AMPERSAND_EQUAL)                                                                               \
  X(CARET)                                                                                         \
  X(CARET_EQUAL)                                                                                   \
  X(LSHIFT)                                                                                        \
  X(LSHIFT_EQUAL)                                                                                  \
  X(RSHIFT)                                                                                        \
  X(RSHIFT_EQUAL)                                                                                  \
  X(TILDE)                                                                                         \
  X(GREATER)                                                                                       \
  X(LESS)                                                                                          \
  X(GREATER_EQUAL)                                                                                 \
  X(LESS_EQUAL)                                                                                    \
  X(EQUAL)                                                                                         \
  X(EXCLAM)                                                                                        \
  X(NOT_EQUAL)                                                                                     \
  X(PLUS)                                                                                          \
  X(MINUS)                                                                                         \
  X(STAR)                                                                                          \
  X(SLASH)                                                                                         \
  X(AND)                                                                                           \
  X(AND_EQUAL)                                                                                     \
  X(OR)                                                                                            \
  X(OR_EQUAL)                                                                                      \
  X(QUESTION_NO_W)                                                                                 \
  X(EXCLAM_NO_W)                                                                                   \
  X(COLON)                                                                                         \
  X(LEFT_PAREN_NO_W)                                                                               \
  X(LEFT_PAREN)                                                                                    \
  X(RIGHT_PAREN)                                                                                   \
  X(LEFT_BRACKET_NO_W)                                                                             \
  X(DOT)                                                                                           \
  X(DOT_NO_W)                                                                                      \
  X(LEFT_BRACKET)                                                                                  \
  X(RIGHT_BRACKET)                                                                                 \
  X(ARROW)                                                                                         \
  X(LEFT_BRACE)                                                                                    \
  X(RIGHT_BRACE)                                                                                   \
  X(SEMICOLON)                                                                                     \
  X(COMMA)                                                                                         \
  X(NUMBER)                                                                                        \
  X(NUMBER_NO_W)                                                                                   \
  X(IDENTIFIER)                                                                                    \
  X(IDENTIFIER_NO_W)                                                                               \
  X(DOUBLE_COLON_NO_W)                                                                             \
  X(AT)                                                                                            \
  X(STRING_LITERAL)                                                                                \
  X(MULTILINE_STRING_LITERAL)                                                                      \
  X(RAW_STRING_LITERAL)                                                                            \
  X(RAW_MULTILINE_STRING_LITERAL)                                                                  \
  X(END_OF_FILE)

namespace amelia {

class AbstractString;
class RuntimeError;

enum class TokenType {
#define X(name) name,
  TOKEN_TYPE_LIST
#undef X
};

inline void token_type_to_string(AbstractString &out, TokenType type) {
  switch (type) {
#define X(name)                                                                                    \
  case TokenType::name:                                                                            \
    out.append(#name);                                                                             \
    break;
    TOKEN_TYPE_LIST
#undef X
  default:
    throw RuntimeError("Invalid TokenType");
  }
}

} // namespace amelia
