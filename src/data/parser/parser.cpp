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
    while (peek().type != terminator) {
      auto stmt_token = peek();
      stmts.push_back(parse_statement());

      if (stmts.size() > 1) {
        enforce_statement_separator(stmts[stmts.size() - 2], stmts[stmts.size() - 1]);
      }
    }
  }

  NodeId parse_introductory_bindings() {
    auto start_token = peek();
    List<NodeId> stmts;
    while (peek().type == TokenType::KEYWORD_LET || peek().type == TokenType::KEYWORD_CONST ||
           peek().type == TokenType::SEMICOLON) {
      auto next_token = peek();
      if (next_token.type == TokenType::KEYWORD_LET) {
        stmts.push_back(parse_let_statement());
      } else if (next_token.type == TokenType::KEYWORD_CONST) {
        stmts.push_back(parse_const_statement());
      } else {
        stmts.push_back(parse_empty_statement());
      }

      if (stmts.size() > 1) {
        enforce_statement_separator(stmts[stmts.size() - 2], stmts[stmts.size() - 1]);
      }
    }
    return m_output.add_node(start_token.loc, IntroductoryBindingsNode{std::move(stmts)});
  }

  void enforce_statement_separator(NodeId left, NodeId right) {
    const Node &stmt = m_output.get_node(left);
    const Node &next_stmt = m_output.get_node(right);
    if (stmt.type() != NodeType::EmptyStatementNode &&
        next_stmt.type() != NodeType::EmptyStatementNode &&
        stmt.location().line == next_stmt.location().line) {
      throw_parser_error_at_location(
          next_stmt.location(),
          "Multiple statements on the same line must be separated by a semicolon"
      );
    }
  }

  void enforce_separator_after_introductory_bindings(NodeId bindings_node_id, NodeId next_stmt_id) {
    const auto &bindings_node = m_output.get_node(bindings_node_id).as_IntroductoryBindingsNode();
    if (bindings_node.bindings.size() > 0) {
      enforce_statement_separator(
          bindings_node.bindings[bindings_node.bindings.size() - 1], next_stmt_id
      );
    }
  }

  NodeId parse_statement() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_LET:
      return parse_let_statement();
    case TokenType::KEYWORD_CONST:
      return parse_const_statement();
    case TokenType::DOUBLE_PLUS:
    case TokenType::DOUBLE_PLUS_NO_W:
      return parse_pre_increment_statement();
    case TokenType::DOUBLE_MINUS:
    case TokenType::DOUBLE_MINUS_NO_W:
      return parse_pre_decrement_statement();
    case TokenType::LEFT_BRACE:
      return parse_block_statement();
    case TokenType::KEYWORD_IF:
      return parse_if_statement();
    case TokenType::KEYWORD_SWITCH:
      return parse_switch_statement();
    case TokenType::KEYWORD_TRY:
      return parse_try_statement();
    case TokenType::KEYWORD_THROW:
      return parse_throw_statement();
    case TokenType::KEYWORD_FOR:
      return parse_for_in_statement();
    case TokenType::KEYWORD_WHILE:
      return parse_while_statement();
    case TokenType::KEYWORD_LABEL:
      return parse_label_statement();
    case TokenType::KEYWORD_GOTO:
      return parse_goto_statement();
    case TokenType::KEYWORD_CONTINUE:
      return parse_continue_statement();
    case TokenType::KEYWORD_RETURN:
      return parse_return_statement();
    case TokenType::KEYWORD_FUN:
      if (peek(1).type == TokenType::IDENTIFIER) {
        return parse_function_declaration();
      }
      break;
    case TokenType::KEYWORD_TYPE:
      return parse_type_declaration();
    case TokenType::SEMICOLON:
      return parse_empty_statement();
    default:
      break;
    }
    return parse_expression_statement();
  }

  NodeId parse_try_statement() {
    auto try_token = next();
    NodeId try_block = parse_statement();
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      clauses.push_back(parse_try_statement_catch_clause());
    }
    return m_output.add_node(try_token.loc, TryStatementNode{try_block, std::move(clauses)});
  }

  NodeId parse_try_statement_catch_clause() {
    auto catch_token = next();
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
    NodeId exc_type = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_statement();
    if (var.has_value()) {
      return m_output.add_node(
          catch_token.loc, CatchClauseBindingNode{var.value(), exc_type, body}
      );
    } else {
      return m_output.add_node(catch_token.loc, CatchClauseNode{exc_type, body});
    }
  }

  NodeId parse_switch_statement() {
    auto switch_token = next();
    read_left_paren("Expected '(' after 'switch'");
    auto introductory_bindings = parse_introductory_bindings();
    NodeId expr = parse_expression();
    enforce_separator_after_introductory_bindings(introductory_bindings, expr);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch statement");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch statement body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_switch_statement_case_clause());
    }

    Option<NodeId> default_body;
    if (peek().type == TokenType::KEYWORD_DEFAULT) {
      ++m_token_index; // consume the 'default' keyword
      default_body = parse_statement();
    }

    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch statement body");
    return m_output.add_node(
        switch_token.loc,
        SwitchStatementNode{introductory_bindings, expr, std::move(clauses), default_body}
    );
  }

  NodeId parse_switch_statement_case_clause() {
    auto case_token = next();
    read_left_paren("Expected '(' after 'case'");
    NodeId introductory_bindings = parse_introductory_bindings();
    List<NodeId> exprs;
    do {
      exprs.push_back(parse_expression());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    } while (peek().type != TokenType::RIGHT_PAREN);
    enforce_separator_after_introductory_bindings(introductory_bindings, exprs[0]);
    ++m_token_index; // consume the right paren
    NodeId body = parse_statement();
    return m_output.add_node(
        case_token.loc, CaseClauseNode{introductory_bindings, std::move(exprs), body}
    );
  }

  NodeId parse_type_declaration() {
    auto type_token = next();
    auto name = parse_expression();
    read_token_type(TokenType::ASSIGN, "Expected '=' after type name in type declaration");
    NodeId type_expr = parse_expression();
    return m_output.add_node(type_token.loc, TypeDeclarationNode{name, type_expr});
  }

  NodeId parse_continue_statement() {
    auto continue_token = next();
    return m_output.add_node(continue_token.loc, ContinueStatementNode{});
  }

  NodeId parse_return_statement() {
    auto return_token = next();
    Option<NodeId> expr;
    auto next_token = peek();
    if (next_token.loc.line == return_token.loc.line && next_token.type != TokenType::SEMICOLON &&
        next_token.type != TokenType::END_OF_FILE && next_token.type != TokenType::RIGHT_BRACE) {
      expr = parse_expression();
    }
    return m_output.add_node(return_token.loc, ReturnStatementNode{expr});
  }

  NodeId parse_function_declaration() {
    auto fun_token = next();
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
    auto start_position = peek().loc;

    List<NodeId> parameters = parse_function_parameter_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    Option<NodeId> capture_list = try_parse_function_signature_capture_annotation_list();
    Option<NodeId> return_type = try_parse_function_return_type();

    return m_output.add_node(
        start_position,
        FunctionSignatureNode{
            std::move(parameters), implicit_parameter_list, capture_list, return_type
        }
    );
  }

  List<NodeId> parse_function_parameter_list() {
    read_left_paren("Expected '(' at the beginning of function parameter list");
    List<NodeId> parameters;
    while (peek().type != TokenType::RIGHT_PAREN) {
      parameters.push_back(parse_function_parameter());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
      }
    }
    ++m_token_index; // consume the ')' token
    return parameters;
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
      type = parse_expression();
    }

    Option<NodeId> default_value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      default_value = parse_expression();
    }

    return m_output.add_node(
        start_position, FunctionParameterNode{variadic, name, type, default_value}
    );
  }

  Option<NodeId> try_parse_implicit_parameter_list() {
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
      return Some(m_output.add_node(
          next_token.loc, FunctionImplicitParameterListNode{std::move(implicit_parameters)}
      ));
    }
    return None();
  }

  Option<NodeId> try_parse_function_signature_capture_annotation_list() {
    auto next_token = peek();
    if (next_token.type == TokenType::LEFT_BRACKET ||
        next_token.type == TokenType::LEFT_BRACKET_NO_W) {
      ++m_token_index; // consume the '[' token
      List<NodeId> capture_annotations;
      while (peek().type != TokenType::RIGHT_BRACKET) {
        capture_annotations.push_back(parse_function_signature_capture_annotation());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        }
      }
      ++m_token_index; // consume the ']' token
      return Some(m_output.add_node(
          next_token.loc, FunctionSignatureCaptureAnnotationListNode{std::move(capture_annotations)}
      ));
    }
    return None();
  }

  NodeId parse_function_signature_capture_annotation() {
    auto start_position = peek().loc;
    FunctionCaptureKind kind;
    auto token = next();
    if (token.type == TokenType::AMPERSAND) {
      kind = FunctionCaptureKind::Ref;
    } else if (token.type == TokenType::KEYWORD_MOVE) {
      kind = FunctionCaptureKind::Move;
    } else if (token.type == TokenType::KEYWORD_COPY) {
      kind = FunctionCaptureKind::Copy;
    } else {
      throw_parser_error(
          token.id, "Expected capture annotation to start with '&', 'move', or 'copy'"
      );
    }
    NodeId var = expect_identifier("Expected variable name in function capture annotation");
    return m_output.add_node(start_position, FunctionSignatureCaptureAnnotationNode{kind, var});
  }

  Option<NodeId> try_parse_function_return_type() {
    auto next_token = peek();
    if (next_token.type == TokenType::ARROW) {
      ++m_token_index; // consume the '->' token
      return parse_expression();
    }
    return None();
  }

  NodeId parse_label_statement() {
    auto label_token = next();
    NodeId label = expect_identifier("Expected identifier after 'label' keyword in label statement"
    );
    return m_output.add_node(label_token.loc, LabelStatementNode{label});
  }

  NodeId parse_goto_statement() {
    auto goto_token = next();
    NodeId label = expect_identifier("Expected identifier after 'goto' keyword in goto statement");
    return m_output.add_node(goto_token.loc, GotoStatementNode{label});
  }

  NodeId parse_empty_statement() {
    auto semicolon_token = next();
    return m_output.add_node(semicolon_token.loc, EmptyStatementNode{});
  }

  NodeId parse_while_statement() {
    auto while_token = next();
    read_left_paren("Expected '(' after 'while' keyword in while statement");
    NodeId introductory_bindings = parse_introductory_bindings();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_bindings(introductory_bindings, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in while statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        while_token.loc, WhileStatementNode{introductory_bindings, condition, body}
    );
  }

  NodeId parse_for_in_statement() {
    auto for_token = next();
    read_left_paren("Expected '(' after 'for' keyword in for-in statement");
    auto introductory_bindings = parse_introductory_bindings();
    List<NodeId> vars;
    vars.push_back(parse_expression());
    enforce_separator_after_introductory_bindings(introductory_bindings, vars[0]);
    while (peek().type == TokenType::COMMA) {
      ++m_token_index; // consume the ',' token
      vars.push_back(parse_expression());
    }
    read_token_type(TokenType::KEYWORD_IN, "Expected 'in' keyword in for-in statement");
    NodeId iterable = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after iterable in for-in statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        for_token.loc, ForInStatementNode{introductory_bindings, std::move(vars), iterable, body}
    );
  }

  NodeId parse_throw_statement() {
    auto throw_token = next();
    NodeId expression = parse_expression();
    return m_output.add_node(throw_token.loc, ThrowStatementNode{expression});
  }

  NodeId parse_if_statement() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if' keyword in if statement");
    NodeId introductory_bindings = parse_introductory_bindings();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_bindings(introductory_bindings, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in if statement");
    NodeId then_branch = parse_statement();
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' keyword
      else_branch = parse_statement();
    }
    return m_output.add_node(
        if_token.loc, IfStatementNode{introductory_bindings, condition, then_branch, else_branch}
    );
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
    auto token = next();
    NodeId operand = parse_expression();
    return m_output.add_node(token.loc, PreIncrementStatementNode{operand});
  }

  NodeId parse_pre_decrement_statement() {
    auto token = next();
    NodeId operand = parse_expression();
    return m_output.add_node(token.loc, PreDecrementStatementNode{operand});
  }

  NodeId parse_const_statement() {
    auto const_token = next();
    NodeId target = parse_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = parse_expression();
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
    auto let_token = next();
    NodeId target = parse_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = parse_expression();
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
    NodeId expr = parse_expression();
    auto next_token = peek();
    switch (next_token.type) {
    case TokenType::DOUBLE_PLUS_NO_W:
      ++m_token_index; // consume the '++' operator
      return m_output.add_node(expr_token.loc, PostIncrementStatementNode{expr});
    case TokenType::DOUBLE_MINUS_NO_W:
      ++m_token_index; // consume the '--' operator
      return m_output.add_node(expr_token.loc, PostDecrementStatementNode{expr});
    case TokenType::ASSIGN:
      ++m_token_index; // consume the '=' operator
      return m_output.add_node(expr_token.loc, AssignmentStatementNode{expr, parse_expression()});
    case TokenType::PLUS_EQUAL:
      ++m_token_index; // consume the '+=' operator
      return m_output.add_node(expr_token.loc, AddAssignStatementNode{expr, parse_expression()});
    case TokenType::MINUS_EQUAL:
      ++m_token_index; // consume the '-=' operator
      return m_output.add_node(expr_token.loc, SubAssignStatementNode{expr, parse_expression()});
    case TokenType::STAR_EQUAL:
      ++m_token_index; // consume the '*=' operator
      return m_output.add_node(expr_token.loc, MulAssignStatementNode{expr, parse_expression()});
    case TokenType::SLASH_EQUAL:
      ++m_token_index; // consume the '/=' operator
      return m_output.add_node(expr_token.loc, DivAssignStatementNode{expr, parse_expression()});
    case TokenType::PERCENT_EQUAL:
      ++m_token_index; // consume the '%=' operator
      return m_output.add_node(expr_token.loc, ModAssignStatementNode{expr, parse_expression()});
    case TokenType::LSHIFT_EQUAL:
      ++m_token_index; // consume the '<<=' operator
      return m_output.add_node(
          expr_token.loc, LeftShiftAssignStatementNode{expr, parse_expression()}
      );
    case TokenType::RSHIFT_EQUAL:
      ++m_token_index; // consume the '>>=' operator
      return m_output.add_node(
          expr_token.loc, RightShiftAssignStatementNode{expr, parse_expression()}
      );
    case TokenType::AMPERSAND_EQUAL:
      ++m_token_index; // consume the '&=' operator
      return m_output.add_node(
          expr_token.loc, BitwiseAndAssignStatementNode{expr, parse_expression()}
      );
    case TokenType::PIPE_EQUAL:
      ++m_token_index; // consume the '|=' operator
      return m_output.add_node(
          expr_token.loc, BitwiseOrAssignStatementNode{expr, parse_expression()}
      );
    case TokenType::CARET_EQUAL:
      ++m_token_index; // consume the '^=' operator
      return m_output.add_node(
          expr_token.loc, BitwiseXorAssignStatementNode{expr, parse_expression()}
      );
    default:
      return m_output.add_node(expr_token.loc, ExpressionStatementNode{expr});
    }
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
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_descend_expr_await_ref();
      return m_output.add_node(start_location, RefExpressionNode{is_const, expr});
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
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_descend_pos_neg_deref_not_bitnot_ell();
      return m_output.add_node(start_location, DerefExpressionNode{is_const, expr});
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
        NodeId index_expr = parse_expression();
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
        throw std::runtime_error("unreachable");
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
      if (is_start_of_lambda_expression()) {
        return parse_lambda_expression();
      }
      return parse_identifier();
    } else if (next_token.type == TokenType::STRING_LITERAL) {
      return parse_string_literal();
    } else if (next_token.type == TokenType::NUMBER || next_token.type == TokenType::NUMBER_FIELD) {
      return parse_number_literal();
    } else if (next_token.type == TokenType::LEFT_PAREN ||
               next_token.type == TokenType::LEFT_PAREN_NO_W) {
      if (is_start_of_lambda_expression()) {
        return parse_lambda_expression();
      }
      return parse_parenthesized_expression();
    } else if (next_token.type == TokenType::LEFT_BRACKET ||
               next_token.type == TokenType::LEFT_BRACKET_NO_W) {
      return parse_bracket_expression();
    } else if (next_token.type == TokenType::LEFT_BRACE) {
      return parse_brace_expression();
    } else if (next_token.type == TokenType::KEYWORD_IF) {
      return parse_if_expression();
    } else if (next_token.type == TokenType::KEYWORD_TRY) {
      return parse_try_expression();
    } else if (next_token.type == TokenType::KEYWORD_SWITCH) {
      return parse_switch_expression();
    } else if (next_token.type == TokenType::KEYWORD_FUN) {
      return parse_function_expression();
    }
    String err("Expected expression, got token ");
    m_token_formatter.format_token(err, m_token_index);
    throw_parser_error_at_current_location(std::move(err));
  }

  NodeId parse_function_expression() {
    auto fun_token = next();
    NodeId signature = parse_function_signature();
    NodeId body = parse_function_body();
    return m_output.add_node(fun_token.loc, FunctionExpressionNode{signature, body});
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
      operator_node = m_output.add_node(start_location, OperatorIdentAmpersandNode{});
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
      auto type = parse_expression();
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

  NodeId parse_switch_expression() {
    auto switch_token = next();
    read_left_paren("Expected '(' after 'switch'");
    auto introductory_bindings = parse_introductory_bindings();
    NodeId expr = parse_expression();
    enforce_separator_after_introductory_bindings(introductory_bindings, expr);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch expression");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch expression body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_switch_expression_case_clause());
    }
    Option<NodeId> default_body;
    if (peek().type == TokenType::KEYWORD_DEFAULT) {
      ++m_token_index; // consume the 'default' keyword
      default_body = parse_expression();
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch expression body");
    return m_output.add_node(
        switch_token.loc,
        SwitchExpressionNode{introductory_bindings, expr, std::move(clauses), default_body}
    );
  }

  NodeId parse_switch_expression_case_clause() {
    auto case_token = next();
    read_left_paren("Expected '(' after 'case'");
    NodeId introductory_bindings = parse_introductory_bindings();
    List<NodeId> exprs;
    do {
      exprs.push_back(parse_expression());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    } while (peek().type != TokenType::RIGHT_PAREN);
    enforce_separator_after_introductory_bindings(introductory_bindings, exprs[0]);
    ++m_token_index; // consume the right paren
    NodeId body = parse_expression();
    return m_output.add_node(
        case_token.loc, CaseClauseNode{introductory_bindings, std::move(exprs), body}
    );
  }

  NodeId parse_try_expression() {
    auto try_token = next();
    NodeId try_block = parse_expression();
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      clauses.push_back(parse_try_expression_catch_clause());
    }
    return m_output.add_node(try_token.loc, TryExpressionNode{try_block, std::move(clauses)});
  }

  NodeId parse_try_expression_catch_clause() {
    auto catch_token = next();
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
    NodeId exc_type = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_expression();
    if (var.has_value()) {
      return m_output.add_node(
          catch_token.loc, CatchClauseBindingNode{var.value(), exc_type, body}
      );
    } else {
      return m_output.add_node(catch_token.loc, CatchClauseNode{exc_type, body});
    }
  }

  NodeId parse_if_expression() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if'");
    NodeId introductory_bindings = parse_introductory_bindings();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_bindings(introductory_bindings, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' expression");
    NodeId then_branch = parse_expression();
    read_token_type(
        TokenType::KEYWORD_ELSE, "Expected 'else' after then-branch of 'if' expression"
    );
    NodeId else_branch = parse_expression();
    return m_output.add_node(
        if_token.loc, IfExpressionNode{introductory_bindings, condition, then_branch, else_branch}
    );
  }

  NodeId parse_bracket_expression() {
    auto open_bracket = next();
    List<NodeId> exprs;
    parse_comma_separated_expression_list(exprs, TokenType::RIGHT_BRACKET);
    ++m_token_index; // consume the right bracket
    return m_output.add_node(open_bracket.loc, BracketExpressionNode{std::move(exprs)});
  }

  NodeId parse_parenthesized_expression() {
    auto open_paren = next();
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
    auto open_brace = next();
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    ++m_token_index; // consume the right brace
    return m_output.add_node(open_brace.loc, BlockExpressionNode{std::move(stmts)});
  }

  NodeId parse_object_literal() {
    auto open_brace = next();
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      read_dot("Expected dot before field name in object literal");
      auto field_token = peek();
      auto field = expect_identifier("Expected field name immediately after dot in object literal");
      read_token_type(TokenType::ASSIGN, "Expected '=' after field name in object literal");
      NodeId value = parse_expression();
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
      exprs.push_back(parse_expression());
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
    return parse_single_identifier();
  }

  bool is_start_of_lambda_expression() {
    auto next_token = peek();
    if (next_token.type == TokenType::IDENTIFIER || next_token.type == TokenType::IDENTIFIER_NO_W) {
      return peek(1).type == TokenType::ARROW;
    } else if (next_token.type == TokenType::LEFT_PAREN ||
               next_token.type == TokenType::LEFT_PAREN_NO_W) {
      int lookahead = 1;
      next_token = peek(lookahead);
      while (next_token.type != TokenType::RIGHT_PAREN) {
        if (next_token.type == TokenType::END_OF_FILE) {
          return false;
        }

        /*
          In a nutshell - in a parenthesized lambda parameter list, we should only see identifiers
          and commas. The only exception is the colon character, which can appear immediately after
          an identifier. If we see anything else, this isn't a lambda parameter list.
        */

        if (next_token.type == TokenType::IDENTIFIER ||
            next_token.type == TokenType::IDENTIFIER_NO_W) {
          if (peek(lookahead + 1).type == TokenType::COLON) {
            return true;
          }
        } else if (next_token.type != TokenType::COMMA) {
          return false;
        }

        ++lookahead;
        next_token = peek(lookahead);
      }
      return peek(lookahead + 1).type == TokenType::ARROW;
    }
    return false;
  }

  NodeId parse_lambda_expression() {
    auto start_token = peek();
    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      List<NodeId> params = parse_function_parameter_list();
      read_token_type(TokenType::ARROW, "Expected '->' after lambda parameter");
      NodeId body = parse_expression();
      return m_output.add_node(start_token.loc, LambdaExpressionNode{std::move(params), body});
    }

    auto single_param = expect_identifier(
        "Expected identifier or '(' to start lambda parameter list"
    );
    read_token_type(TokenType::ARROW, "Expected '->' after lambda parameter");
    NodeId body = parse_expression();
    return m_output.add_node(start_token.loc, LambdaExpressionNode{{single_param}, body});
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
    NodeId expr = parse_expression();
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
    if (m_token_index >= static_cast<TokenId>(m_input.tokens().size())) {
      throw std::runtime_error("Attempting to read past end of token stream");
    }
    auto token = m_input.get_token(m_token_index);
    TokenWithId result{m_token_index, token.type, token.location};
    ++m_token_index;
    return result;
  }

  TokenWithId peek(TokenId n = 0) const {
    if (m_token_index + n >= static_cast<TokenId>(m_input.tokens().size())) {
      throw std::runtime_error("Attempting to peek past end of token stream");
    }
    auto token = m_input.get_token(m_token_index + n);
    return TokenWithId{m_token_index + n, token.type, token.location};
  }

  [[noreturn]] void throw_parser_error_at_current_location(String message) const {
    throw_parser_error(m_token_index, std::move(message));
  }

  [[noreturn]] void throw_parser_error_at_location(Location location, String message) const {
    throw ParserError(location, std::move(message));
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
