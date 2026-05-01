#include "token_type.h"

#include "prelude.h"

void amelia::token_type_to_string(TokenType type, IString &out) {
  switch (type) {
  case TokenType::IDENTIFIER:
    out.append("IDENTIFIER");
    break;
  case TokenType::KEYWORD_FUN:
    out.append("FUN");
    break;
  case TokenType::KEYWORD_IF:
    out.append("IF");
    break;
  case TokenType::KEYWORD_ELSE:
    out.append("ELSE");
    break;
  case TokenType::KEYWORD_TRY:
    out.append("TRY");
    break;
  case TokenType::KEYWORD_CATCH:
    out.append("CATCH");
    break;
  case TokenType::KEYWORD_STATIC:
    out.append("STATIC");
    break;
  case TokenType::KEYWORD_THIS:
    out.append("THIS");
    break;
  case TokenType::KEYWORD_MODULE:
    out.append("MODULE");
    break;
  case TokenType::KEYWORD_VOID:
    out.append("VOID");
    break;
  case TokenType::KEYWORD_THROW:
    out.append("THROW");
    break;
  case TokenType::KEYWORD_IMPORT:
    out.append("IMPORT");
    break;
  case TokenType::KEYWORD_AS:
    out.append("AS");
    break;
  case TokenType::KEYWORD_SWITCH:
    out.append("SWITCH");
    break;
  case TokenType::KEYWORD_CASE:
    out.append("CASE");
    break;
  case TokenType::KEYWORD_CLASS:
    out.append("CLASS");
    break;
  case TokenType::KEYWORD_UNION:
    out.append("UNION");
    break;
  case TokenType::KEYWORD_RECORD:
    out.append("RECORD");
    break;
  case TokenType::KEYWORD_TYPE:
    out.append("TYPE");
    break;
  case TokenType::KEYWORD_CONCEPT:
    out.append("CONCEPT");
    break;
  case TokenType::KEYWORD_BOOL:
    out.append("BOOL");
    break;
  case TokenType::KEYWORD_AUTO:
    out.append("AUTO");
    break;
  case TokenType::KEYWORD_LET:
    out.append("LET");
    break;
  case TokenType::KEYWORD_CONST:
    out.append("CONST");
    break;
  case TokenType::KEYWORD_IMPL:
    out.append("IMPL");
    break;
  case TokenType::KEYWORD_ANY:
    out.append("ANY");
    break;
  case TokenType::KEYWORD_LABEL:
    out.append("LABEL");
    break;
  case TokenType::KEYWORD_GOTO:
    out.append("GOTO");
    break;
  case TokenType::KEYWORD_ASYNC:
    out.append("ASYNC");
    break;
  case TokenType::KEYWORD_AWAIT:
    out.append("AWAIT");
    break;
  case TokenType::KEYWORD_TRUE:
    out.append("TRUE");
    break;
  case TokenType::KEYWORD_FALSE:
    out.append("FALSE");
    break;
  case TokenType::KEYWORD_NULL:
    out.append("NULL");
    break;
  case TokenType::KEYWORD_DEFAULT:
    out.append("DEFAULT");
    break;
  case TokenType::KEYWORD_OPEN:
    out.append("OPEN");
    break;
  case TokenType::KEYWORD_OVERRIDE:
    out.append("OVERRIDE");
    break;
  case TokenType::KEYWORD_LOCAL:
    out.append("LOCAL");
    break;
  case TokenType::KEYWORD_PUBLIC:
    out.append("PUBLIC");
    break;
  case TokenType::KEYWORD_PRIVATE:
    out.append("PRIVATE");
    break;
  case TokenType::KEYWORD_PROTECTED:
    out.append("PROTECTED");
    break;
  case TokenType::KEYWORD_ENUM:
    out.append("ENUM");
    break;
  case TokenType::KEYWORD_COPY:
    out.append("COPY");
    break;
  case TokenType::KEYWORD_MOVE:
    out.append("MOVE");
    break;
  case TokenType::KEYWORD_OPERATOR:
    out.append("OPERATOR");
    break;
  case TokenType::KEYWORD_EXTERN:
    out.append("EXTERN");
    break;
  case TokenType::KEYWORD_INLINE:
    out.append("INLINE");
    break;
  case TokenType::KEYWORD_DELETE:
    out.append("DELETE");
    break;
  case TokenType::KEYWORD_NEW:
    out.append("NEW");
    break;
  case TokenType::KEYWORD_IMPLICIT:
    out.append("IMPLICIT");
    break;
  case TokenType::KEYWORD_WITH:
    out.append("WITH");
    break;
  case TokenType::KEYWORD_WHEN:
    out.append("WHEN");
    break;
  case TokenType::KEYWORD_RETURN:
    out.append("RETURN");
    break;
  case TokenType::KEYWORD_CONTINUE:
    out.append("CONTINUE");
    break;
  case TokenType::KEYWORD_BREAK:
    out.append("BREAK");
    break;
  case TokenType::KEYWORD_WHILE:
    out.append("WHILE");
    break;
  case TokenType::KEYWORD_FOR:
    out.append("FOR");
    break;
  case TokenType::KEYWORD_IN:
    out.append("IN");
    break;
  case TokenType::KEYWORD_ABSTRACT:
    out.append("ABSTRACT");
    break;
  case TokenType::KEYWORD_SUPER:
    out.append("SUPER");
    break;
  case TokenType::NUMBER:
    out.append("NUMBER");
    break;
  case TokenType::ASSIGN:
    out.append("ASSIGN");
    break;
  case TokenType::PLUS_EQUAL:
    out.append("PLUS_EQUAL");
    break;
  case TokenType::MINUS_EQUAL:
    out.append("MINUS_EQUAL");
    break;
  case TokenType::STAR_EQUAL:
    out.append("STAR_EQUAL");
    break;
  case TokenType::SLASH_EQUAL:
    out.append("SLASH_EQUAL");
    break;
  case TokenType::PERCENT_EQUAL:
    out.append("PERCENT_EQUAL");
    break;
  case TokenType::GREATER:
    out.append("GREATER");
    break;
  case TokenType::LESS:
    out.append("LESS");
    break;
  case TokenType::GREATER_EQUAL:
    out.append("GREATER_EQUAL");
    break;
  case TokenType::LESS_EQUAL:
    out.append("LESS_EQUAL");
    break;
  case TokenType::EQUAL:
    out.append("EQUAL");
    break;
  case TokenType::NOT_EQUAL:
    out.append("NOT_EQUAL");
    break;
  case TokenType::PLUS:
    out.append("PLUS");
    break;
  case TokenType::MINUS:
    out.append("MINUS");
    break;
  case TokenType::STAR:
    out.append("STAR");
    break;
  case TokenType::SLASH:
    out.append("SLASH");
    break;
  case TokenType::AND:
    out.append("AND");
    break;
  case TokenType::OR:
    out.append("OR");
    break;
  case TokenType::NOT:
    out.append("NOT");
    break;
  case TokenType::PERCENT:
    out.append("PERCENT");
    break;
  case TokenType::AMPERSAND:
    out.append("AMPERSAND");
    break;
  case TokenType::PIPE:
    out.append("PIPE");
    break;
  case TokenType::CARET:
    out.append("CARET");
    break;
  case TokenType::PIPE_EQUAL:
    out.append("PIPE_EQUAL");
    break;
  case TokenType::AMPERSAND_EQUAL:
    out.append("AMPERSAND_EQUAL");
    break;
  case TokenType::CARET_EQUAL:
    out.append("CARET_EQUAL");
    break;
  case TokenType::TILDE:
    out.append("TILDE");
    break;
  case TokenType::LEFT_BRACE:
    out.append("LEFT_BRACE");
    break;
  case TokenType::RIGHT_BRACE:
    out.append("RIGHT_BRACE");
    break;
  case TokenType::ARROW:
    out.append("ARROW");
    break;
  case TokenType::SEMICOLON:
    out.append("SEMICOLON");
    break;
  case TokenType::COMMA:
    out.append("COMMA");
    break;
  case TokenType::QUESTION:
    out.append("QUESTION");
    break;
  case TokenType::EXCLAMATION:
    out.append("EXCLAMATION");
    break;
  case TokenType::FIELD_ACCESS:
    out.append("FIELD_ACCESS");
    break;
  case TokenType::DOTTED_IDENTIFIER:
    out.append("DOTTED_IDENTIFIER");
    break;
  case TokenType::NAMESPACE_ACCESS:
    out.append("NAMESPACE_ACCESS");
    break;
  case TokenType::COLON:
    out.append("COLON");
    break;
  case TokenType::FUNCALL_START:
    out.append("FUNCALL_START");
    break;
  case TokenType::LEFT_PAREN:
    out.append("LEFT_PAREN");
    break;
  case TokenType::RIGHT_PAREN:
    out.append("RIGHT_PAREN");
    break;
  case TokenType::IX_START:
    out.append("IX_START");
    break;
  case TokenType::LEFT_BRACKET:
    out.append("LEFT_BRACKET");
    break;
  case TokenType::RIGHT_BRACKET:
    out.append("RIGHT_BRACKET");
    break;
  case TokenType::MACRO_NAME:
    out.append("MACRO_NAME");
    break;
  case TokenType::ANNOTATION_NAME:
    out.append("ANNOTATION_NAME");
    break;
  case TokenType::STRING_LITERAL:
    out.append("STRING_LITERAL");
    break;
  case TokenType::MULTILINE_STRING_LITERAL:
    out.append("MULTILINE_STRING_LITERAL");
    break;
  case TokenType::RAW_STRING_LITERAL:
    out.append("RAW_STRING_LITERAL");
    break;
  case TokenType::RAW_MULTILINE_STRING_LITERAL:
    out.append("RAW_MULTILINE_STRING_LITERAL");
    break;
  case TokenType::END_OF_FILE:
    out.append("END_OF_FILE");
    break;
  default:
    throw RuntimeError("Invalid TokenType");
  }
}
