#include "parser.h"
#include "prelude.h"

#include "data/lexer/lexer_result.h"
#include "data/lexer/token_formatter.h"
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
      : m_output(output), m_input(input), m_token_index(0), m_token_formatter(input) {}

  NodeId parse_module() {
    List<NodeId> top_level_statements;
    auto start_token = peek();
    parse_statements(top_level_statements, TokenType::END_OF_FILE);
    return m_output.add_node(start_token.loc, ModuleNode{std::move(top_level_statements)});
  }

  void parse_statements(List<NodeId> &stmts, TokenType terminator) {
    bool previous_statement_ended_with_semicolon = false;
    while (peek().type != terminator) {
      auto stmt_token = peek();
      stmts.push_back(parse_statement());

      if (!previous_statement_ended_with_semicolon && stmts.size() > 1) {
        const Node &stmt = m_output.get_node(stmts[stmts.size() - 1]);
        const Node &prev_stmt = m_output.get_node(stmts[stmts.size() - 2]);
        if (prev_stmt.type() != NodeType::EmptyStatementNode &&
            prev_stmt.location().line == stmt.location().line) {
          throw_parser_error(
              stmt_token.id, "Multiple statements on the same line must be separated by a semicolon"
          );
        }
      }

      previous_statement_ended_with_semicolon = peek(-1).type == TokenType::SEMICOLON;
    }
  }

  NodeId parse_statement() {
    auto token = peek();
    NodeId result;
    bool is_empty = false;
    if (token.type == TokenType::KEYWORD_LET) {
      result = parse_let_statement();
    } else if (token.type == TokenType::KEYWORD_CONST) {
      result = parse_const_statement();
    } else if (token.type == TokenType::DOUBLE_PLUS || token.type == TokenType::DOUBLE_PLUS_NO_W) {
      result = parse_pre_increment_statement();
    } else if (token.type == TokenType::DOUBLE_MINUS ||
               token.type == TokenType::DOUBLE_MINUS_NO_W) {
      result = parse_pre_decrement_statement();
    } else if (token.type == TokenType::LEFT_BRACE) {
      return parse_block_statement();
    } else if (token.type == TokenType::KEYWORD_IF) {
      result = parse_if_statement();
    } else if (token.type == TokenType::KEYWORD_THROW) {
      result = parse_throw_statement();
    } else if (token.type == TokenType::KEYWORD_FOR) {
      result = parse_for_in_statement();
    } else if (token.type == TokenType::KEYWORD_WHILE) {
      result = parse_while_statement();
    } else if (token.type == TokenType::KEYWORD_LABEL) {
      result = parse_label_statement();
    } else if (token.type == TokenType::KEYWORD_GOTO) {
      result = parse_goto_statement();
    } else if (token.type == TokenType::KEYWORD_CONTINUE) {
      result = parse_continue_statement();
    } else if (token.type == TokenType::KEYWORD_RETURN) {
      result = parse_return_statement();
    } else if (token.type == TokenType::KEYWORD_FUN && peek(1).type == TokenType::IDENTIFIER) {
      result = parse_function_declaration();
    } else if (token.type == TokenType::SEMICOLON) {
      result = parse_empty_statement();
      is_empty = true;
    } else {
      result = parse_expression_statement();
    }
    if (!is_empty && peek().type == TokenType::SEMICOLON) {
      ++m_token_index; // consume one trailing ';' token
    }
    return result;
  }

  NodeId parse_continue_statement() {
    auto continue_token = next();
    return m_output.add_node(continue_token.loc, ContinueStatementNode{});
  }

  NodeId parse_return_statement() {
    auto return_token = next();
    NodeId expr = -1;
    if (peek().loc.line == return_token.loc.line) {
      expr = try_parse_expression();
    }
    if (expr == -1) {
      return m_output.add_node(return_token.loc, ReturnStatementNode{});
    }
    return m_output.add_node(return_token.loc, ReturnValueStatementNode{expr});
  }

  NodeId parse_function_declaration() {
    auto fun_token = next(); // consume the 'fun' keyword
    auto name = expect_identifier("Expected function name after 'fun' keyword");
    NodeId signature = parse_function_signature();
    Option<NodeId> body;
    if (peek().type == TokenType::LEFT_BRACE) {
      body = parse_function_body();
    }
    return m_output.add_node(
        fun_token.loc, FunctionDeclarationStatementNode{name, signature, body}
    );
  }

  NodeId parse_function_body() {
    auto left_brace_token = read_token_type(
        TokenType::LEFT_BRACE, "Expected '{' at the beginning of function body"
    );
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    ++m_token_index; // consume the '}' token
    return m_output.add_node(left_brace_token.loc, FunctionBodyNode{std::move(stmts)});
  }

  NodeId parse_function_signature() {
    auto start_position = read_left_paren("Expected '(' at the beginning of function signature")
                              .loc;
    List<NodeId> parameters;
    while (peek().type != TokenType::RIGHT_PAREN) {
      parameters.push_back(parse_function_parameter());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
      }
    }
    ++m_token_index; // consume the ')' token
    Option<NodeId> implicit_parameter_list;
    auto next_token = peek();
    if (next_token.type == TokenType::KEYWORD_WITH) {
      ++m_token_index; // consume the 'with' keyword
      read_left_paren("Expected '(' after 'with' keyword in function signature");
      List<NodeId> implicit_parameters;
      while (peek().type != TokenType::RIGHT_PAREN) {
        implicit_parameters.push_back(parse_function_parameter());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        }
      }
      ++m_token_index; // consume the ')' token
      implicit_parameter_list = m_output.add_node(
          next_token.loc, FunctionImplicitParameterListNode{std::move(implicit_parameters)}
      );
    }

    Option<NodeId> return_type;
    if (peek().type == TokenType::ARROW) {
      ++m_token_index; // consume the '->' token
      return_type = require_expression();
    }

    return m_output.add_node(
        start_position,
        FunctionSignatureNode{std::move(parameters), implicit_parameter_list, return_type}
    );
  }

  NodeId parse_function_parameter() {
    auto start_position = peek().loc;
    bool variadic = false;
    if (peek().type == TokenType::ELLIPSIS) {
      variadic = true;
      ++m_token_index; // consume the '...' token
    }
    auto name = expect_identifier("Expected parameter name in function signature");
    Option<NodeId> type;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type = require_expression();
    }

    Option<NodeId> default_value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      default_value = require_expression();
    }

    return m_output.add_node(
        start_position, FunctionParameterNode{variadic, name, type, default_value}
    );
  }

  NodeId parse_label_statement() {
    auto label_token = next(); // consume the 'label' keyword
    NodeId label = expect_identifier("Expected identifier after 'label' keyword in label statement"
    );
    return m_output.add_node(label_token.loc, LabelStatementNode{label});
  }

  NodeId parse_goto_statement() {
    auto goto_token = next(); // consume the 'goto' keyword
    NodeId label = expect_identifier("Expected identifier after 'goto' keyword in goto statement");
    return m_output.add_node(goto_token.loc, GotoStatementNode{label});
  }

  NodeId parse_empty_statement() {
    auto semicolon_token = next(); // consume the ';' token
    return m_output.add_node(semicolon_token.loc, EmptyStatementNode{});
  }

  NodeId parse_while_statement() {
    auto while_token = next(); // consume the 'while' keyword
    read_left_paren("Expected '(' after 'while' keyword in while statement");
    NodeId condition = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in while statement");
    NodeId body = parse_statement();
    return m_output.add_node(while_token.loc, WhileStatementNode{condition, body});
  }

  NodeId parse_for_in_statement() {
    auto for_token = next(); // consume the 'for' keyword
    read_left_paren("Expected '(' after 'for' keyword in for-in statement");
    List<NodeId> vars;
    vars.push_back(require_expression());
    while (peek().type == TokenType::COMMA) {
      ++m_token_index; // consume the ',' token
      vars.push_back(require_expression());
    }
    read_token_type(TokenType::KEYWORD_IN, "Expected 'in' keyword in for-in statement");
    NodeId iterable = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after iterable in for-in statement");
    NodeId body = parse_statement();
    return m_output.add_node(for_token.loc, ForInStatementNode{std::move(vars), iterable, body});
  }

  NodeId parse_throw_statement() {
    auto throw_token = next(); // consume the 'throw' keyword
    NodeId expression = require_expression();
    return m_output.add_node(throw_token.loc, ThrowStatementNode{expression});
  }

  NodeId parse_if_statement() {
    auto if_token = next(); // consume the 'if' keyword
    read_left_paren("Expected '(' after 'if' keyword in if statement");
    NodeId condition = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in if statement");
    NodeId then_branch = parse_statement();
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' keyword
      NodeId else_branch = parse_statement();
      return m_output.add_node(
          if_token.loc, IfThenElseStatementNode{condition, then_branch, else_branch}
      );
    } else {
      return m_output.add_node(if_token.loc, IfThenStatementNode{condition, then_branch});
    }
  }

  NodeId parse_block_statement() {
    auto left_brace_token = read_token_type(
        TokenType::LEFT_BRACE, "Expected '{' at the beginning of block statement"
    );
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' at the end of block statement");
    return m_output.add_node(left_brace_token.loc, BlockStatementNode{std::move(stmts)});
  }

  NodeId parse_pre_increment_statement() {
    auto token = next(); // consume the '++' operator
    NodeId operand = require_expression();
    return m_output.add_node(token.loc, PreIncrementStatementNode{operand});
  }

  NodeId parse_pre_decrement_statement() {
    auto token = next(); // consume the '--' operator
    NodeId operand = require_expression();
    return m_output.add_node(token.loc, PreDecrementStatementNode{operand});
  }

  NodeId parse_const_statement() {
    auto const_token = next(); // consume the 'const' keyword
    NodeId target = require_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = require_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = require_expression();
    }
    if (type_annotation.has_value()) {
      if (expression.has_value()) {
        return m_output.add_node(
            const_token.loc,
            ConstAssignAnnotationNode{target, type_annotation.value(), expression.value()}
        );
      } else {
        return m_output.add_node(
            const_token.loc, ConstAnnotationNode{target, type_annotation.value()}
        );
      }
    } else if (expression.has_value()) {
      return m_output.add_node(
          const_token.loc, ConstAssignmentStatementNode{target, expression.value()}
      );
    } else {
      return m_output.add_node(const_token.loc, ConstStatementNode{target});
    }
  }

  NodeId parse_let_statement() {
    auto let_token = next(); // consume the 'let' keyword
    NodeId target = require_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = require_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = require_expression();
    }
    if (type_annotation.has_value()) {
      if (expression.has_value()) {
        return m_output.add_node(
            let_token.loc,
            LetAssignAnnotationNode{target, type_annotation.value(), expression.value()}
        );
      } else {
        return m_output.add_node(let_token.loc, LetAnnotationNode{target, type_annotation.value()});
      }
    } else if (expression.has_value()) {
      return m_output.add_node(
          let_token.loc, LetAssignmentStatementNode{target, expression.value()}
      );
    } else {
      return m_output.add_node(let_token.loc, LetStatementNode{target});
    }
  }

  NodeId parse_expression_statement() {
    auto expr_token = peek();
    NodeId expr = require_expression();
    auto next_token = peek();
    if (next_token.type == TokenType::DOUBLE_PLUS_NO_W) {
      ++m_token_index; // consume the '++' operator
      return m_output.add_node(expr_token.loc, PostIncrementStatementNode{expr});
    } else if (next_token.type == TokenType::DOUBLE_MINUS_NO_W) {
      ++m_token_index; // consume the '--' operator
      return m_output.add_node(expr_token.loc, PostDecrementStatementNode{expr});
    } else if (next_token.type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, AssignmentStatementNode{expr, value});
    } else if (next_token.type == TokenType::PLUS_EQUAL) {
      ++m_token_index; // consume the '+=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, AddAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::MINUS_EQUAL) {
      ++m_token_index; // consume the '-=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, SubAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::STAR_EQUAL) {
      ++m_token_index; // consume the '*=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, MulAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::SLASH_EQUAL) {
      ++m_token_index; // consume the '/=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, DivAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::PERCENT_EQUAL) {
      ++m_token_index; // consume the '%=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, ModAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::LSHIFT_EQUAL) {
      ++m_token_index; // consume the '<<=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, LeftShiftAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::RSHIFT_EQUAL) {
      ++m_token_index; // consume the '>>=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, RightShiftAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::AMPERSAND_EQUAL) {
      ++m_token_index; // consume the '&=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, BitwiseAndAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::PIPE_EQUAL) {
      ++m_token_index; // consume the '|=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, BitwiseOrAssignStatementNode{expr, value});
    } else if (next_token.type == TokenType::CARET_EQUAL) {
      ++m_token_index; // consume the '^=' operator
      NodeId value = require_expression();
      return m_output.add_node(expr_token.loc, BitwiseXorAssignStatementNode{expr, value});
    }
    return m_output.add_node(expr_token.loc, ExpressionStatementNode{expr});
  }

  NodeId require_expression() {
    NodeId result = parse_descend_expr_or();
    if (result == -1) {
      String err("Expected expression, got token ");
      m_token_formatter.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
    }
    return result;
  }

  NodeId try_parse_expression() {
    return parse_descend_expr_or();
  }

  NodeId parse_descend_expr_or() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_and();
    while (peek().type == TokenType::OR) {
      ++m_token_index; // consume the '||' operator
      NodeId right = parse_descend_expr_and();
      left = m_output.add_node(start_location, OrExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_and() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_or();
    while (peek().type == TokenType::AND) {
      ++m_token_index; // consume the '&&' operator
      NodeId right = parse_descend_expr_bitwise_or();
      left = m_output.add_node(start_location, AndExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_or() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_xor();
    while (peek().type == TokenType::PIPE) {
      ++m_token_index; // consume the '|' operator
      NodeId right = parse_descend_expr_bitwise_xor();
      left = m_output.add_node(start_location, BitwiseOrExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_xor() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_bitwise_and();
    while (peek().type == TokenType::CARET) {
      ++m_token_index; // consume the '^' operator
      NodeId right = parse_descend_expr_bitwise_and();
      left = m_output.add_node(start_location, BitwiseXorExpressionNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_and() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_eq_ne();
    while (peek().type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      NodeId right = parse_descend_expr_eq_ne();
      left = m_output.add_node(start_location, BitwiseAndExpressionNode{left, right});
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
        left = m_output.add_node(start_location, EqualsExpressionNode{left, right});
      } else {
        left = m_output.add_node(start_location, NotEqualsExpressionNode{left, right});
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
        left = m_output.add_node(start_location, GreaterExpressionNode{left, right});
      } else if (next_token.type == TokenType::LESS) {
        left = m_output.add_node(start_location, LessExpressionNode{left, right});
      } else if (next_token.type == TokenType::GREATER_EQUAL) {
        left = m_output.add_node(start_location, GreaterEqualsExpressionNode{left, right});
      } else {
        left = m_output.add_node(start_location, LessEqualsExpressionNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_lshift_rshift() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_add_node();
    auto next_token = peek();
    while (next_token.type == TokenType::LSHIFT || next_token.type == TokenType::RSHIFT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_add_node();
      if (next_token.type == TokenType::LSHIFT) {
        left = m_output.add_node(start_location, LeftShiftExpressionNode{left, right});
      } else {
        left = m_output.add_node(start_location, RightShiftExpressionNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_add_node() {
    auto start_location = peek().loc;
    NodeId left = parse_descend_expr_mul_div_mod();
    auto next_token = peek();
    while (next_token.type == TokenType::PLUS || next_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_mul_div_mod();
      if (next_token.type == TokenType::PLUS) {
        left = m_output.add_node(start_location, AddExpressionNode{left, right});
      } else {
        left = m_output.add_node(start_location, SubtractExpressionNode{left, right});
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
        left = m_output.add_node(start_location, MultiplyExpressionNode{left, right});
      } else if (next_token.type == TokenType::SLASH) {
        left = m_output.add_node(start_location, DivideExpressionNode{left, right});
      } else {
        left = m_output.add_node(start_location, ModuloExpressionNode{left, right});
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
      return m_output.add_node(start_location, AwaitExpressionNode{expr});
    } else if (next_token.type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      NodeId expr = parse_descend_expr_await_ref();
      return m_output.add_node(start_location, RefExpressionNode{expr});
    }
    return parse_descend_pos_neg_deref_not_bitnot_ell();
  }

  NodeId parse_descend_pos_neg_deref_not_bitnot_ell() {
    auto next_token = peek();
    auto start_location = next_token.loc;
    if (next_token.type == TokenType::PLUS) {
      ++m_token_index; // consume the '+' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, PositiveExpressionNode{expr});
    } else if (next_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the '-' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, NegativeExpressionNode{expr});
    } else if (next_token.type == TokenType::TILDE) {
      ++m_token_index; // consume the '~' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, BitwiseNotExpressionNode{expr});
    } else if (next_token.type == TokenType::STAR) {
      ++m_token_index; // consume the '*' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, DerefExpressionNode{expr});
    } else if (next_token.type == TokenType::EXCLAM || next_token.type == TokenType::EXCLAM_NO_W) {
      ++m_token_index; // consume the '!' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, NotExpressionNode{expr});
    } else if (next_token.type == TokenType::ELLIPSIS) {
      ++m_token_index; // consume the '...' operator
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, EllipsisExpressionNode{expr});
    }
    return parse_descend_expr_field_ix_funcall();
  }

  NodeId parse_descend_expr_field_ix_funcall() {
    auto start_location = peek().loc;
    auto left = parse_descend_expr_scope_resolution();
    auto next_token = peek();
    while (next_token.type == TokenType::DOT_NO_W || next_token.type == TokenType::NUMBER_FIELD ||
           next_token.type == TokenType::LEFT_BRACKET_NO_W ||
           next_token.type == TokenType::LEFT_PAREN_NO_W) {
      if (next_token.type == TokenType::DOT_NO_W) {
        ++m_token_index; // consume the '.' operator
        auto next_type = peek().type;
        if (next_type != TokenType::IDENTIFIER_NO_W && next_type != TokenType::KEYWORD_OPERATOR) {
          throw_parser_error_at_current_location(
              "Expected identifier immediately after '.' in field access expression"
          );
        }
        left = m_output.add_node(
            start_location, FieldAccessExpressionNode{left, parse_identifier()}
        );
      } else if (next_token.type == TokenType::NUMBER_FIELD) {
        m_token_index++; // consume the numeric field token
        left = m_output.add_node(
            start_location, NumericFieldAccessExpressionNode{left, next_token.id}
        );
      } else if (next_token.type == TokenType::LEFT_BRACKET_NO_W) {
        ++m_token_index; // consume the '[' operator
        NodeId index_expr = require_expression();
        read_token_type(
            TokenType::RIGHT_BRACKET, "Expected ']' after index expression in index access"
        );
        left = m_output.add_node(start_location, IndexingExpressionNode{left, index_expr});
      } else if (next_token.type == TokenType::LEFT_PAREN_NO_W) {
        ++m_token_index; // consume the '(' operator
        List<NodeId> args;
        while (peek().type != TokenType::RIGHT_PAREN) {
          args.push_back(parse_function_call_argument());
          if (peek().type == TokenType::COMMA) {
            ++m_token_index; // consume the comma
          }
        }
        ++m_token_index; // consume the ')' operator
        left = m_output.add_node(start_location, FunctionCallExpressionNode{left, std::move(args)});
      } else {
        throw RuntimeError("unreachable");
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_scope_resolution() {
    auto start_location = peek().loc;
    NodeId left = parse_atom();
    while (peek().type == TokenType::DOUBLE_COLON_NO_W) {
      ++m_token_index; // consume the '::' operator
      auto next_type = peek().type;
      if (next_type != TokenType::IDENTIFIER_NO_W && next_type != TokenType::KEYWORD_OPERATOR) {
        throw_parser_error_at_current_location(
            "Expected identifier immediately after '::' in scope resolution expression"
        );
      }
      left = m_output.add_node(
          start_location, ScopeResolutionExpressionNode{left, parse_identifier()}
      );
    }
    return left;
  }

  NodeId parse_atom() {
    auto next_token = peek();
    if (next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W ||
        next_token.type == TokenType::KEYWORD_OPERATOR) {
      return parse_identifier();
    } else if (next_token.type == TokenType::STRING_LITERAL) {
      return parse_string_literal();
    } else if (next_token.type == TokenType::NUMBER || next_token.type == TokenType::NUMBER_FIELD) {
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
      return parse_if_then_else_expression();
    } else if (next_token.type == TokenType::KEYWORD_TRY) {
      return parse_try_catch_expression();
    } else if (next_token.type == TokenType::KEYWORD_SWITCH) {
      return parse_switch_expression();
    } else {
      return -1;
    }
  }

  NodeId parse_operator_ident() {
    auto start_location = next().loc; // consume the 'operator' token
    NodeId operator_node;
    switch (next().type) {
    case TokenType::PLUS:
      operator_node = m_output.add_node(start_location, OperatorIdentAddNode{});
      break;
    case TokenType::MINUS:
      operator_node = m_output.add_node(start_location, OperatorIdentSubNode{});
      break;
    case TokenType::STAR:
      operator_node = m_output.add_node(start_location, OperatorIdentStarNode{});
      break;
    case TokenType::SLASH:
      operator_node = m_output.add_node(start_location, OperatorIdentDivNode{});
      break;
    case TokenType::PERCENT:
      operator_node = m_output.add_node(start_location, OperatorIdentModNode{});
      break;
    case TokenType::DOUBLE_PLUS:
    case TokenType::DOUBLE_PLUS_NO_W:
      operator_node = m_output.add_node(start_location, OperatorIdentIncNode{});
      break;
    case TokenType::DOUBLE_MINUS:
    case TokenType::DOUBLE_MINUS_NO_W:
      operator_node = m_output.add_node(start_location, OperatorIdentDecNode{});
      break;
    case TokenType::EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentEqNode{});
      break;
    case TokenType::NOT_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentNeqNode{});
      break;
    case TokenType::GREATER:
      operator_node = m_output.add_node(start_location, OperatorIdentGtNode{});
      break;
    case TokenType::LESS:
      operator_node = m_output.add_node(start_location, OperatorIdentLtNode{});
      break;
    case TokenType::GREATER_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentGteNode{});
      break;
    case TokenType::LESS_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentLteNode{});
      break;
    case TokenType::EXCLAM:
    case TokenType::EXCLAM_NO_W:
      operator_node = m_output.add_node(start_location, OperatorIdentNotNode{});
      break;
    case TokenType::AND:
      operator_node = m_output.add_node(start_location, OperatorIdentAndNode{});
      break;
    case TokenType::OR:
      operator_node = m_output.add_node(start_location, OperatorIdentOrNode{});
      break;
    case TokenType::TILDE:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseNotNode{});
      break;
    case TokenType::AMPERSAND:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseAndNode{});
      break;
    case TokenType::PIPE:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseOrNode{});
      break;
    case TokenType::CARET:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseXorNode{});
      break;
    case TokenType::LSHIFT:
      operator_node = m_output.add_node(start_location, OperatorIdentLeftShiftNode{});
      break;
    case TokenType::RSHIFT:
      operator_node = m_output.add_node(start_location, OperatorIdentRightShiftNode{});
      break;
    case TokenType::ASSIGN:
      operator_node = m_output.add_node(start_location, OperatorIdentAssignNode{});
      break;
    case TokenType::PLUS_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentAddAssignNode{});
      break;
    case TokenType::MINUS_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentSubAssignNode{});
      break;
    case TokenType::STAR_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentMulAssignNode{});
      break;
    case TokenType::SLASH_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentDivAssignNode{});
      break;
    case TokenType::PERCENT_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentModAssignNode{});
      break;
    case TokenType::AMPERSAND_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseAndAssignNode{});
      break;
    case TokenType::PIPE_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseOrAssignNode{});
      break;
    case TokenType::CARET_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentBitwiseXorAssignNode{});
      break;
    case TokenType::LSHIFT_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentLeftShiftAssignNode{});
      break;
    case TokenType::RSHIFT_EQUAL:
      operator_node = m_output.add_node(start_location, OperatorIdentRightShiftAssignNode{});
      break;
    case TokenType::LEFT_BRACKET:
    case TokenType::LEFT_BRACKET_NO_W:
      read_token_type(TokenType::RIGHT_BRACKET, "Expected ']' following 'operator['");
      operator_node = m_output.add_node(start_location, OperatorIdentIxNode{});
      break;
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_PAREN_NO_W:
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' following 'operator('");
      operator_node = m_output.add_node(start_location, OperatorIdentFuncallNode{});
      break;
    case TokenType::KEYWORD_AS: {
      read_left_bracket("Expected '[' after 'operator as'");
      auto type = require_expression();
      read_token_type(
          TokenType::RIGHT_BRACKET, "Expected ']' after type in 'operator as' identifier"
      );
      operator_node = m_output.add_node(start_location, OperatorIdentAsNode{type});
      break;
    }
    default:
      throw_parser_error_at_current_location("Expected operator after 'operator' keyword");
    }
    return m_output.add_node(start_location, OperatorIdentifierNode{operator_node});
  }

  NodeId parse_scope_resolution() {
    auto start_location = peek().loc;
    auto left = parse_single_identifier();
    while (peek().type == TokenType::DOUBLE_COLON_NO_W) {
      m_token_index++; // consume the '::' operator
      if (peek().type != TokenType::IDENTIFIER_NO_W) {
        throw_parser_error_at_current_location(
            "Expected identifier immediately after '::' in scope resolution expression"
        );
      }
      auto name = parse_single_identifier();
      left = m_output.add_node(start_location, ScopeResolutionExpressionNode{left, name});
    }
    return left;
  }

  NodeId parse_switch_expression() {
    auto switch_token = next(); // consume the 'switch' keyword
    read_left_paren("Expected '(' after 'switch'");
    NodeId expr = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch expression");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch expression body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_case_clause());
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch expression body");
    return m_output.add_node(switch_token.loc, SwitchExpressionNode{expr, std::move(clauses)});
  }

  NodeId parse_case_clause() {
    auto case_token = next(); // consume the 'case' keyword
    read_left_paren("Expected '(' after 'case'");
    NodeId expr = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after case clause condition");
    NodeId body = require_expression();
    return m_output.add_node(case_token.loc, CaseClauseNode{expr, body});
  }

  NodeId parse_try_catch_expression() {
    auto try_token = next(); // consume the 'try' keyword
    NodeId try_block = require_expression();
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      clauses.push_back(parse_catch_clause());
    }
    return m_output.add_node(try_token.loc, TryCatchExpressionNode{try_block, std::move(clauses)});
  }

  NodeId parse_catch_clause() {
    auto catch_token = next(); // consume the 'catch' keyword
    read_left_paren("Expected '(' after 'catch'");
    Option<NodeId> var;
    auto next_token = peek();
    auto following_token = peek(1);
    if ((next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W
        ) &&
        following_token.type == TokenType::COLON) {
      var = parse_identifier();
      ++m_token_index; // consume the ':' token
    }
    NodeId exc_type = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = require_expression();
    if (var.has_value()) {
      return m_output.add_node(
          catch_token.loc, CatchClauseBindingNode{var.value(), exc_type, body}
      );
    } else {
      return m_output.add_node(catch_token.loc, CatchClauseNode{exc_type, body});
    }
  }

  NodeId parse_if_then_else_expression() {
    auto if_token = next(); // consume the 'if' keyword
    read_left_paren("Expected '(' after 'if'");
    NodeId condition = require_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' expression");
    NodeId then_branch = require_expression();
    read_token_type(
        TokenType::KEYWORD_ELSE, "Expected 'else' after then-branch of 'if' expression"
    );
    NodeId else_branch = require_expression();
    return m_output.add_node(
        if_token.loc, IfThenElseExpressionNode{condition, then_branch, else_branch}
    );
  }

  NodeId parse_array_literal() {
    auto open_bracket = next(); // consume the left bracket
    List<NodeId> exprs;
    parse_comma_separated_expression_list(exprs, TokenType::RIGHT_BRACKET);
    ++m_token_index; // consume the right bracket
    return m_output.add_node(open_bracket.loc, ArrayLiteralNode{std::move(exprs)});
  }

  NodeId parse_parenthesized_expression() {
    auto open_paren = next(); // consume the left paren
    List<NodeId> exprs;
    parse_comma_separated_expression_list(exprs, TokenType::RIGHT_PAREN);
    ++m_token_index; // consume the right paren
    return m_output.add_node(open_paren.loc, ParenthesizedExpressionNode{std::move(exprs)});
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
    return m_output.add_node(open_brace.loc, BlockExpressionNode{std::move(stmts)});
  }

  NodeId parse_object_literal() {
    auto open_brace = next(); // consume the left brace
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      read_dot("Expected dot before field name in object literal");
      auto field_token = peek();
      auto field = expect_identifier("Expected field name immediately after dot in object literal");
      read_token_type(TokenType::ASSIGN, "Expected '=' after field name in object literal");
      NodeId value = require_expression();
      entries.push_back(m_output.add_node(field_token.loc, KeyValueEntryNode{field, value}));
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    }
    ++m_token_index; // consume the right brace
    return m_output.add_node(open_brace.loc, ObjectLiteralNode{std::move(entries)});
  }

  void parse_comma_separated_expression_list(List<NodeId> &exprs, TokenType terminator) {
    while (peek().type != terminator) {
      exprs.push_back(require_expression());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index;
      }
    }
  }

  NodeId parse_string_literal() {
    auto token = next();
    return m_output.add_node(token.loc, StringLiteralNode{token.id});
  }

  NodeId parse_number_literal() {
    auto token = next();
    return m_output.add_node(token.loc, NumberLiteralNode{token.id});
  }

  NodeId expect_identifier(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::IDENTIFIER && token.type != TokenType::IDENTIFIER_NO_W &&
        token.type != TokenType::KEYWORD_OPERATOR) {
      throw_parser_error(token.id, String(error_message));
    }
    return parse_identifier();
  }

  NodeId parse_identifier() {
    if (peek().type == TokenType::KEYWORD_OPERATOR) {
      return parse_operator_ident();
    }
    if (peek().type == TokenType::IDENTIFIER || peek().type == TokenType::IDENTIFIER_NO_W) {
      return parse_single_identifier();
    }
    throw_parser_error_at_current_location("Expected identifier");
  }

  NodeId parse_single_identifier() {
    auto ident = next();
    return m_output.add_node(ident.loc, IdentifierNode{ident.id});
  }

  NodeId parse_function_call_argument() {
    Option<NodeId> name;
    auto next_token = peek();
    if ((next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W
        ) &&
        peek(1).type == TokenType::ASSIGN) {
      name = parse_identifier();
      ++m_token_index; // consume the '=' token
    }
    NodeId expr = require_expression();
    if (name.has_value()) {
      return m_output.add_node(next_token.loc, NamedFunctionArgumentNode{name.value(), expr});
    }
    return m_output.add_node(next_token.loc, PositionalFunctionArgumentNode{expr});
  }

private:
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

  TokenWithId read_left_bracket(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::LEFT_BRACKET && token.type != TokenType::LEFT_BRACKET_NO_W) {
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

  TokenWithId peek(TokenId n = 0) const {
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
  TokenId m_token_index;
  TokenFormatter m_token_formatter;
};
} // namespace

NodeId Parser::parse_module(ParserResult &output, const LexerResult &input) {
  ParserState state(output, input);
  return state.parse_module();
}

} // namespace amelia
