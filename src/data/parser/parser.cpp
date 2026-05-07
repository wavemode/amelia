#include "parser.h"
#include "prelude.h"

#include "data/lexer/lexer_result.h"
#include "data/parser/parser_error.h"
#include "data/parser/parser_result.h"

namespace amelia {

namespace {

struct TokenWithId {
  TokenId id;
  TokenType type;
  Location loc;
};

class ParserState {
public:
  ParserState(ParserResult &output, const LexerResult &input)
      : m_output(output), m_input(input), m_token_index(0) {}

  NodeId parse_module() {
    List<NodeId> top_level_statements;
    auto start_token = peek();
    parse_statements(top_level_statements, TokenType::END_OF_FILE);
    return m_output.add_ModuleNode(start_token.loc, ModuleNode{std::move(top_level_statements)});
  }

  void parse_statements(List<NodeId> &stmts, TokenType terminator) {
    while (peek().type != terminator) {
      if (peek().type == TokenType::SEMICOLON) {
        ++m_token_index;
        continue;
      }
      auto next_token = peek();
      stmts.push_back(parse_statement());
      if (stmts.size() > 1) {
        auto prev_stmt_info = m_output.get_node_info(stmts[stmts.size() - 2]);
        auto next_stmt_info = m_output.get_node_info(stmts[stmts.size() - 1]);
        if (prev_stmt_info.location.line == next_stmt_info.location.line) {
          throw_parser_error(next_token.id, "Multiple statements on the same line are not allowed");
        }
      }
    }
  }

  NodeId parse_statement() {
    auto token = peek();
    if (token.type == TokenType::KEYWORD_LET) {
      return parse_let_statement();
    } else if (token.type == TokenType::KEYWORD_CONST) {
      return parse_const_statement();
    } else {
      return parse_expression_statement();
    }
  }

  NodeId parse_const_statement() {
    auto const_token = next(); // consume the 'const' keyword
    NodeId target = parse_expression();
    auto assign_token = read_token_type(
        TokenType::ASSIGN, "Expected '=' after identifier in const statement"
    );
    NodeId expression = parse_expression();
    return m_output.add_ConstStatementNode(const_token.loc, ConstStatementNode{target, expression});
  }

  NodeId parse_let_statement() {
    auto let_token = next(); // consume the 'let' keyword
    NodeId target = parse_expression();
    auto assign_token = read_token_type(
        TokenType::ASSIGN, "Expected '=' after identifier in let statement"
    );
    NodeId expression = parse_expression();
    return m_output.add_LetStatementNode(let_token.loc, LetStatementNode{target, expression});
  }

  NodeId parse_expression_statement() {
    auto expr_token = peek();
    NodeId expr = parse_expression();
    return m_output.add_ExpressionStatementNode(expr_token.loc, ExpressionStatementNode{expr});
  }

  NodeId parse_expression() {
    return parse_descend_expr_or();
  }

  NodeId parse_descend_expr_or() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_and();
    while (peek().type == TokenType::OR) {
      ++m_token_index; // consume the '||' operator
      NodeId right = parse_descend_expr_and();
      left = m_output.add_OrExpressionNode(start_location, OrExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_and() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_or();
    while (peek().type == TokenType::AND) {
      ++m_token_index; // consume the '&&' operator
      NodeId right = parse_descend_expr_bitwise_or();
      left = m_output.add_AndExpressionNode(start_location, AndExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_or() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_xor();
    while (peek().type == TokenType::PIPE) {
      ++m_token_index; // consume the '|' operator
      NodeId right = parse_descend_expr_bitwise_xor();
      left = m_output.add_BitwiseOrExpressionNode(
          start_location, BitwiseOrExpressionNode{left, right}
      );
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_xor() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_and();
    while (peek().type == TokenType::CARET) {
      ++m_token_index; // consume the '^' operator
      NodeId right = parse_descend_expr_bitwise_and();
      left = m_output.add_BitwiseXorExpressionNode(
          start_location, BitwiseXorExpressionNode{left, right}
      );
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_and() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_eq_ne();
    while (peek().type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      NodeId right = parse_descend_expr_eq_ne();
      left = m_output.add_BitwiseAndExpressionNode(
          start_location, BitwiseAndExpressionNode{left, right}
      );
    }
    return left;
  }

  NodeId parse_descend_expr_eq_ne() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_gt_lt();
    auto next_token = peek();
    while (next_token.type == TokenType::EQUAL || next_token.type == TokenType::NOT_EQUAL) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_gt_lt();
      if (next_token.type == TokenType::EQUAL) {
        left = m_output.add_EqualsExpressionNode(start_location, EqualsExpressionNode{left, right});
      } else {
        left = m_output.add_NotEqualsExpressionNode(
            start_location, NotEqualsExpressionNode{left, right}
        );
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_gt_lt() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_lshift_rshift();
    auto next_token = peek();
    while (next_token.type == TokenType::GREATER || next_token.type == TokenType::LESS ||
           next_token.type == TokenType::GREATER_EQUAL ||
           next_token.type == TokenType::LESS_EQUAL) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_lshift_rshift();
      if (next_token.type == TokenType::GREATER) {
        left = m_output.add_GreaterExpressionNode(
            start_location, GreaterExpressionNode{left, right}
        );
      } else if (next_token.type == TokenType::LESS) {
        left = m_output.add_LessExpressionNode(start_location, LessExpressionNode{left, right});
      } else if (next_token.type == TokenType::GREATER_EQUAL) {
        left = m_output.add_GreaterEqualsExpressionNode(
            start_location, GreaterEqualsExpressionNode{left, right}
        );
      } else {
        left = m_output.add_LessEqualsExpressionNode(
            start_location, LessEqualsExpressionNode{left, right}
        );
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_lshift_rshift() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_add_sub();
    auto next_token = peek();
    while (next_token.type == TokenType::LSHIFT || next_token.type == TokenType::RSHIFT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_add_sub();
      if (next_token.type == TokenType::LSHIFT) {
        left = m_output.add_LeftShiftExpressionNode(
            start_location, LeftShiftExpressionNode{left, right}
        );
      } else {
        left = m_output.add_RightShiftExpressionNode(
            start_location, RightShiftExpressionNode{left, right}
        );
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_add_sub() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_mul_div_mod();
    auto next_token = peek();
    while (next_token.type == TokenType::PLUS || next_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_mul_div_mod();
      if (next_token.type == TokenType::PLUS) {
        left = m_output.add_AddExpressionNode(start_location, AddExpressionNode{left, right});
      } else {
        left = m_output.add_SubtractExpressionNode(
            start_location, SubtractExpressionNode{left, right}
        );
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_mul_div_mod() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_await_ref();
    auto next_token = peek();
    while (next_token.type == TokenType::STAR || next_token.type == TokenType::SLASH ||
           next_token.type == TokenType::PERCENT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_await_ref();
      if (next_token.type == TokenType::STAR) {
        left = m_output.add_MultiplyExpressionNode(
            start_location, MultiplyExpressionNode{left, right}
        );
      } else if (next_token.type == TokenType::SLASH) {
        left = m_output.add_DivideExpressionNode(start_location, DivideExpressionNode{left, right});
      } else {
        left = m_output.add_ModuloExpressionNode(start_location, ModuloExpressionNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_await_ref() {
    auto next_token = peek();
    auto start_location = next_token.loc;
    if (next_token.type == TokenType::KEYWORD_AWAIT) {
      ++m_token_index; // consume the 'await' keyword
      NodeId expr = parse_descend_expr_await_ref();
      return m_output.add_AwaitExpressionNode(start_location, AwaitExpressionNode{expr});
    } else if (next_token.type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      NodeId expr = parse_descend_expr_await_ref();
      return m_output.add_RefExpressionNode(start_location, RefExpressionNode{expr});
    }
    return parse_descend_pos_neg_deref_not_bitnot_ell();
  }

  NodeId parse_descend_pos_neg_deref_not_bitnot_ell() {
    auto next_token = peek();
    auto start_location = next_token.loc;
    if (next_token.type == TokenType::PLUS) {
      ++m_token_index; // consume the '+' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_PositiveExpressionNode(start_location, PositiveExpressionNode{expr});
    } else if (next_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the '-' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_NegativeExpressionNode(start_location, NegativeExpressionNode{expr});
    } else if (next_token.type == TokenType::TILDE) {
      ++m_token_index; // consume the '~' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_BitwiseNotExpressionNode(start_location, BitwiseNotExpressionNode{expr});
    } else if (next_token.type == TokenType::STAR) {
      ++m_token_index; // consume the '*' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_DerefExpressionNode(start_location, DerefExpressionNode{expr});
    } else if (next_token.type == TokenType::EXCLAM || next_token.type == TokenType::EXCLAM_NO_W) {
      ++m_token_index; // consume the '!' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_NotExpressionNode(start_location, NotExpressionNode{expr});
    } else if (next_token.type == TokenType::ELLIPSIS) {
      ++m_token_index; // consume the '...' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_EllipsisExpressionNode(start_location, EllipsisExpressionNode{expr});
    }
    return parse_descend_expr_field_access();
  }

  NodeId parse_descend_expr_field_access() {
    auto start_location = peek().loc;
    auto left = parse_atom();
    auto next_token = peek();
    while (next_token.type == TokenType::DOT_NO_W || token_is_number_field(next_token)) {
      if (next_token.type == TokenType::DOT_NO_W) {
        ++m_token_index; // consume the '.' operator
        if (peek().type != TokenType::IDENTIFIER_NO_W) {
          throw_parser_error_at_current_location(
              "Expected identifier immediately after '.' in field access expression"
          );
        }
        left = m_output.add_FieldAccessExpressionNode(
            start_location, FieldAccessExpressionNode{left, parse_identifier()}
        );
      } else {
        m_token_index++; // consume the numeric field token
        left = m_output.add_NumericFieldAccessExpressionNode(
            start_location, NumericFieldAccessExpressionNode{left, next_token.id}
        );
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_atom() {
    auto next_token = peek();
    if (next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W) {
      return parse_identifier();
    } else if (next_token.type == TokenType::STRING_LITERAL) {
      return parse_string_literal();
    } else if (next_token.type == TokenType::NUMBER || next_token.type == TokenType::NUMBER_NO_W) {
      return parse_number_literal();
    } else if (next_token.type == TokenType::LEFT_PAREN ||
               next_token.type == TokenType::LEFT_PAREN_NO_W) {
      return parse_parenthesized_expression();
    } else if (next_token.type == TokenType::LEFT_BRACKET ||
               next_token.type == TokenType::LEFT_BRACKET_NO_W) {
      return parse_array_literal();
    } else if (next_token.type == TokenType::LEFT_BRACE) {
      return parse_brace_expression();
    } else if (next_token.type == TokenType::KEYWORD_IF) {
      return parse_if_expression();
    } else if (next_token.type == TokenType::KEYWORD_TRY) {
      return parse_try_catch_expression();
    } else if (next_token.type == TokenType::KEYWORD_SWITCH) {
      return parse_switch_expression();
    } else {
      String err("Expected expression, got token ");
      m_input.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
    }
  }

  NodeId parse_switch_expression() {
    auto switch_token = next(); // consume the 'switch' keyword
    read_left_paren("Expected '(' after 'switch'");
    NodeId expr = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch expression");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch expression body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_case_clause());
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch expression body");
    return m_output.add_SwitchExpressionNode(
        switch_token.loc, SwitchExpressionNode{expr, std::move(clauses)}
    );
  }

  NodeId parse_case_clause() {
    auto case_token = next(); // consume the 'case' keyword
    read_left_paren("Expected '(' after 'case'");
    NodeId expr = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after case clause condition");
    NodeId body = parse_expression();
    return m_output.add_CaseClauseNode(case_token.loc, CaseClauseNode{expr, body});
  }

  NodeId parse_try_catch_expression() {
    auto try_token = next(); // consume the 'try' keyword
    NodeId try_block = parse_expression();
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      clauses.push_back(parse_catch_clause());
    }
    return m_output.add_TryCatchExpressionNode(
        try_token.loc, TryCatchExpressionNode{try_block, std::move(clauses)}
    );
  }

  NodeId parse_catch_clause() {
    auto catch_token = next(); // consume the 'catch' keyword
    read_left_paren("Expected '(' after 'catch'");
    Option<TokenId> var;
    auto next_token = peek();
    auto following_token = peek(1);
    if ((next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W
        ) &&
        following_token.type == TokenType::COLON) {
      var = next_token.id;
      m_token_index += 2; // consume the identifier and the colon
    }
    NodeId exc_type = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_expression();
    return m_output.add_CatchClauseNode(catch_token.loc, CatchClauseNode{var, exc_type, body});
  }

  NodeId parse_if_expression() {
    auto if_token = next(); // consume the 'if' keyword
    read_left_paren("Expected '(' after 'if'");
    NodeId condition = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' expression");
    NodeId then_branch = parse_expression();
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index;
      else_branch = parse_expression();
    }
    return m_output.add_IfExpressionNode(
        if_token.loc, IfExpressionNode{condition, then_branch, else_branch}
    );
  }

  NodeId parse_array_literal() {
    auto open_bracket = next(); // consume the left bracket
    List<NodeId> exprs;
    parse_comma_separated_expression_list(exprs, TokenType::RIGHT_BRACKET);
    ++m_token_index; // consume the right bracket
    return m_output.add_ArrayLiteralNode(open_bracket.loc, ArrayLiteralNode{std::move(exprs)});
  }

  NodeId parse_parenthesized_expression() {
    auto open_paren = next(); // consume the left paren
    List<NodeId> exprs;
    parse_comma_separated_expression_list(exprs, TokenType::RIGHT_PAREN);
    ++m_token_index; // consume the right paren
    return m_output.add_ParenthesizedExpressionNode(
        open_paren.loc, ParenthesizedExpressionNode{std::move(exprs)}
    );
  }

  NodeId parse_brace_expression() {
    auto next_token = peek(1);
    if (next_token.type == TokenType::RIGHT_BRACE || next_token.type == TokenType::DOT) {
      return parse_object_literal();
    }
    return parse_block_expression();
  }

  NodeId parse_block_expression() {
    auto open_brace = next(); // consume the left brace
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    ++m_token_index; // consume the right brace
    return m_output.add_BlockExpressionNode(open_brace.loc, BlockExpressionNode{std::move(stmts)});
  }

  NodeId parse_object_literal() {
    auto open_brace = next(); // consume the left brace
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      read_dot("Expected dot before field name in object literal");
      auto field_token = read_token_type(
          TokenType::IDENTIFIER_NO_W, "Expected field name immediately after dot in object literal"
      );
      read_token_type(TokenType::ASSIGN, "Expected '=' after field name in object literal");
      NodeId value = parse_expression();
      entries.push_back(
          m_output.add_KeyValueEntryNode(field_token.loc, KeyValueEntryNode{field_token.id, value})
      );
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma and continue parsing entries
      }
    }
    ++m_token_index; // consume the right brace
    return m_output.add_ObjectLiteralNode(open_brace.loc, ObjectLiteralNode{std::move(entries)});
  }

  void parse_comma_separated_expression_list(List<NodeId> &exprs, TokenType terminator) {
    while (peek().type != terminator) {
      exprs.push_back(parse_expression());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index;
      }
    }
  }

  NodeId parse_string_literal() {
    auto token = next();
    return m_output.add_StringLiteralNode(token.loc, StringLiteralNode{token.id});
  }

  NodeId parse_number_literal() {
    auto token = next();
    return m_output.add_NumberLiteralNode(token.loc, NumberLiteralNode{token.id});
  }

  NodeId parse_identifier() {
    auto ident = next();
    return m_output.add_IdentifierNode(ident.loc, IdentifierNode{ident.id});
  }

private:
  bool token_is_number_field(TokenWithId token) {
    // number must have no whitespace before it in the source
    if (token.type != TokenType::NUMBER_NO_W) {
      return false;
    }

    auto lit = m_input.get_number_literal(token.id);
    return (
        // number must have the form `.1` (only a decimal point and some digits after)
        lit.base_prefix.size() == 0 && lit.exponent_digits.size() == 0 &&
        lit.exponent_sign.size() == 0 && lit.exponent_prefix.size() == 0 &&
        lit.integer_digits.size() == 0 && lit.has_decimal_point && lit.fractional_digits.size() > 0
    );
  }

  TokenWithId read_dot(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::DOT && token.type != TokenType::DOT_NO_W) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId read_left_paren(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::LEFT_PAREN && token.type != TokenType::LEFT_PAREN_NO_W) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId read_token_type(TokenType expected, Text error_message) {
    auto token = peek();
    if (token.type != expected) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId next() {
    if (m_token_index >= m_input.tokens().size()) {
      throw RuntimeError("Attempting to read past end of token stream");
    }
    auto token = m_input.get_token(m_token_index);
    TokenWithId result{m_token_index, token.type, token.location};
    ++m_token_index;
    return result;
  }

  TokenWithId peek(size_t n = 0) const {
    if (m_token_index + n >= m_input.tokens().size()) {
      throw RuntimeError("Attempting to peek past end of token stream");
    }
    auto token = m_input.get_token(m_token_index + n);
    return TokenWithId{m_token_index + n, token.type, token.location};
  }

  [[noreturn]] void throw_parser_error_at_current_location(String message) const {
    throw_parser_error(m_token_index, std::move(message));
  }

  [[noreturn]] void throw_parser_error(TokenId token_id, String message) const {
    auto token = m_input.get_token(token_id);
    throw ParserError(token.location, std::move(message));
  }

  ParserResult &m_output;
  const LexerResult &m_input;
  size_t m_token_index;
};
} // namespace

NodeId Parser::parse_module(ParserResult &output, const LexerResult &input) {
  ParserState state(output, input);
  return state.parse_module();
}

} // namespace amelia
