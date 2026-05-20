#include "parser.h"
#include "prelude.h"

#include "data/lexer/lexer_result.h"
#include "data/lexer/token_formatter.h"
#include "data/parser/parser_error.h"
#include "data/parser/parser_result.h"

namespace amelia {

namespace {

bool is_identifier(TokenType type) {
  return type == TokenType::IDENTIFIER || type == TokenType::IDENTIFIER_NO_W ||
         type == TokenType::QUOTED_IDENTIFIER || type == TokenType::QUOTED_IDENTIFIER_NO_W;
}

bool is_identifier_no_w(TokenType type) {
  return type == TokenType::IDENTIFIER_NO_W || type == TokenType::QUOTED_IDENTIFIER_NO_W;
}

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
    List<NodeId> decls;
    auto start_token = peek();
    parse_top_level_declarations(decls, TokenType::END_OF_FILE);
    return m_output.add_node(start_token.loc, ModuleNode{std::move(decls)});
  }

  void parse_top_level_declarations(List<NodeId> &decls, TokenType terminator) {
    while (peek().type != terminator) {
      auto decl = try_parse_top_level_declaration();
      if (!decl.has_value()) {
        throw_parser_error_at_current_location("Expected declaration");
      }
      decls.push_back(decl.value());

      if (decls.size() > 1) {
        enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
      }
    }
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

  List<NodeId> parse_introductory_decls() {
    auto start_token = peek();
    List<NodeId> decls;
    while (true) {
      auto decl = try_parse_local_declaration();
      if (!decl.has_value()) {
        break;
      }
      decls.push_back(decl.value());
      if (decls.size() > 1) {
        enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
      }
    }
    return decls;
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

  void enforce_separator_after_introductory_decls(const List<NodeId> &decls, NodeId next_stmt_id) {
    if (decls.size() > 0) {
      enforce_statement_separator(decls[decls.size() - 1], next_stmt_id);
    }
  }

  Option<NodeId> try_parse_top_level_declaration() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_LOCAL:
      return parse_local_declaration();
    default:
      break;
    }
    return try_parse_declaration();
  }

  Option<NodeId> try_parse_local_declaration() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_IMPLICIT:
      return parse_implicit_declaration();
    default:
      break;
    }
    return try_parse_declaration();
  }

  Option<NodeId> try_parse_declaration() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_LET:
      return parse_let_declaration();
    case TokenType::KEYWORD_CONST:
      return parse_const_declaration();
    case TokenType::KEYWORD_FUN: {
      if (is_identifier(peek(1).type)) {
        return parse_function_declaration();
      }
    } break;
    case TokenType::KEYWORD_CLASS:
      return parse_class_declaration();
    case TokenType::KEYWORD_TYPE:
      return parse_type_declaration();
    case TokenType::KEYWORD_OPERATOR:
      return parse_operator_function_declaration();
    case TokenType::SEMICOLON:
      return parse_empty_statement();
    default:
      break;
    }
    return None();
  }

  NodeId expect_declaration(Text error_message) {
    auto decl = try_parse_declaration();
    if (!decl.has_value()) {
      throw_parser_error_at_current_location(String(error_message));
    }
    return decl.value();
  }

  NodeId expect_local_declaration(Text error_message) {
    auto decl = try_parse_local_declaration();
    if (!decl.has_value()) {
      throw_parser_error_at_current_location(String(error_message));
    }
    return decl.value();
  }

  NodeId parse_statement() {
    auto token = peek();
    switch (token.type) {
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
    default:
      break;
    }
    auto decl = try_parse_local_declaration();
    if (decl.has_value()) {
      return decl.value();
    }
    return parse_expression_statement();
  }

  NodeId parse_implicit_declaration() {
    auto implicit_token = next();
    auto next_token = peek();
    if (next_token.type != TokenType::KEYWORD_LET && next_token.type != TokenType::KEYWORD_CONST &&
        next_token.type != TokenType::KEYWORD_FUN) {
      throw_parser_error_at_current_location(
          "Expected 'let', 'const', or 'fun' declaration after 'implicit' keyword"
      );
    }
    NodeId decl = try_parse_declaration().value();
    return m_output.add_node(implicit_token.loc, ImplicitDeclarationNode{decl});
  }

  NodeId parse_operator_function_declaration() {
    auto operator_token = peek();
    auto operator_ident = parse_operator_ident();
    auto signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(
        operator_token.loc, OperatorFunctionDeclarationNode{operator_ident, signature, body}
    );
  }

  NodeId parse_local_declaration() {
    auto local_token = next();
    NodeId decl = expect_declaration("Expected declaration after 'local' keyword");
    return m_output.add_node(local_token.loc, VisibilityNode{DeclarationVisibility::Local, decl});
  }

  NodeId parse_class_declaration() {
    auto class_token = next();
    auto name = expect_identifier("Expected class name after 'class' keyword");
    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    Option<NodeId> base_class_list = try_parse_base_class_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start class body");
    List<NodeId> decls;
    while (peek().type != TokenType::RIGHT_BRACE) {
      decls.push_back(parse_class_body_declaration());
      if (decls.size() > 1) {
        enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
      }
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end class body");
    return m_output.add_node(
        class_token.loc,
        ClassDeclarationNode{
            name, generic_parameter_list, base_class_list, implicit_parameter_list, std::move(decls)
        }
    );
  }

  Option<NodeId> try_parse_base_class_list() {
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      List<NodeId> base_classes;
      base_classes.push_back(
          expect_identifier("Expected base class name after ':' in class declaration")
      );
      while (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
        base_classes.push_back(parse_type_expression());
      }
      return Some(m_output.add_node(peek(-1).loc, ClassBaseClassListNode{std::move(base_classes)}));
    }
    return None();
  }

  NodeId parse_class_body_declaration() {
    auto next_token = peek();
    switch (next_token.type) {
    case TokenType::KEYWORD_STATIC:
      return parse_class_static_declaration();
    case TokenType::KEYWORD_CONST:
      return parse_class_const_declaration();
    case TokenType::KEYWORD_COPY:
      return parse_class_copy_declaration();
    case TokenType::KEYWORD_MOVE: {
      return parse_class_move_declaration();
    }
    case TokenType::IDENTIFIER:
    case TokenType::IDENTIFIER_NO_W:
    case TokenType::QUOTED_IDENTIFIER:
    case TokenType::QUOTED_IDENTIFIER_NO_W: {
      auto following_token = peek(1);
      if (following_token.type == TokenType::LEFT_PAREN ||
          following_token.type == TokenType::LEFT_PAREN_NO_W ||
          following_token.type == TokenType::LEFT_BRACKET ||
          following_token.type == TokenType::LEFT_BRACKET_NO_W) {
        return parse_constructor_declaration();
      }
      return parse_class_field();
    } break;
    case TokenType::KEYWORD_PUBLIC:
    case TokenType::KEYWORD_PRIVATE:
    case TokenType::KEYWORD_PROTECTED:
    case TokenType::KEYWORD_LOCAL:
      return parse_class_body_visibility_declaration();
    default:
      break;
    }
    return expect_local_declaration("Expected declaration in class body");
  }

  NodeId parse_class_body_visibility_declaration() {
    auto visibility_token = next();
    DeclarationVisibility visibility;
    switch (visibility_token.type) {
    case TokenType::KEYWORD_PUBLIC:
      visibility = DeclarationVisibility::Public;
      break;
    case TokenType::KEYWORD_PRIVATE:
      visibility = DeclarationVisibility::Private;
      break;
    case TokenType::KEYWORD_PROTECTED:
      visibility = DeclarationVisibility::Protected;
      break;
    case TokenType::KEYWORD_LOCAL:
      visibility = DeclarationVisibility::Local;
      break;
    default:
      throw std::runtime_error("Invalid visibility modifier");
    }
    NodeId decl = parse_class_body_declaration();
    return m_output.add_node(visibility_token.loc, VisibilityNode{visibility, decl});
  }

  NodeId parse_constructor_declaration() {
    auto name_token = peek();
    NodeId name;
    if (name_token.type == TokenType::KEYWORD_COPY) {
      name = m_output.add_node(name_token.loc, CopyCtorNameNode{});
      ++m_token_index; // consume the 'copy' keyword
    } else if (name_token.type == TokenType::KEYWORD_MOVE) {
      name = m_output.add_node(name_token.loc, MoveCtorNameNode{});
      ++m_token_index; // consume the 'move' keyword
    } else {
      name = expect_identifier("Expected constructor name");
    }
    NodeId signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(name_token.loc, ClassConstructorNode{name, signature, body});
  }

  NodeId parse_class_static_declaration() {
    auto static_token = next();
    NodeId decl = parse_class_body_declaration();
    return m_output.add_node(static_token.loc, ClassStaticDeclarationNode{decl});
  }

  NodeId parse_class_const_declaration() {
    auto const_token = next();
    NodeId decl = parse_class_body_declaration();
    return m_output.add_node(const_token.loc, ClassConstDeclarationNode{decl});
  }

  NodeId parse_class_copy_declaration() {
    auto copy_token = next();
    NodeId decl = parse_class_body_declaration();
    return m_output.add_node(copy_token.loc, ClassCopyDeclarationNode{decl});
  }

  NodeId parse_class_move_declaration() {
    auto move_token = next();
    NodeId decl = parse_class_body_declaration();
    return m_output.add_node(move_token.loc, ClassMoveDeclarationNode{decl});
  }

  NodeId parse_class_field() {
    auto start_location = peek().loc;
    auto name = expect_identifier("Expected field name in class declaration");
    Option<NodeId> type;
    Option<NodeId> initializer;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type = parse_type_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      initializer = parse_expression();
    }
    return m_output.add_node(start_location, ClassFieldNode{name, type, initializer});
  }

  Option<NodeId> try_parse_generic_parameter_list() {
    auto start_token = peek();
    if (start_token.type == TokenType::LEFT_BRACKET_NO_W ||
        start_token.type == TokenType::LEFT_BRACKET) {
      ++m_token_index; // consume the left bracket
      List<NodeId> parameters;
      do {
        parameters.push_back(parse_generic_parameter());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the comma
        }
      } while (peek().type != TokenType::RIGHT_BRACKET);
      read_token_type(TokenType::RIGHT_BRACKET, "Expected ']' to end generic parameter list");

      Option<List<NodeId>> additional_constraints;
      if (peek().type == TokenType::KEYWORD_WHEN) {
        ++m_token_index; // consume the 'when' keyword
        read_left_paren("Expected '(' after 'when' in generic parameter list");
        List<NodeId> constraints;
        do {
          constraints.push_back(parse_type_constraint());
          if (peek().type == TokenType::COMMA) {
            ++m_token_index; // consume the comma
          }
        } while (peek().type != TokenType::RIGHT_PAREN);
        read_token_type(
            TokenType::RIGHT_PAREN,
            "Expected ')' to end additional constraints in generic parameter list"
        );
        additional_constraints = std::move(constraints);
      }

      return Some(m_output.add_node(
          start_token.loc,
          GenericParameterListNode{std::move(parameters), std::move(additional_constraints)}
      ));
    }
    return None();
  }

  NodeId parse_generic_parameter() {
    auto start_token = peek();
    bool is_const = false;
    if (start_token.type == TokenType::KEYWORD_CONST) {
      is_const = true;
      ++m_token_index; // consume the 'const' keyword
    }
    NodeId type = expect_identifier("Expected type name for generic parameter");
    Option<NodeId> constraint;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      constraint = parse_type_expression();
    }
    return m_output.add_node(start_token.loc, GenericParameterNode{is_const, type, constraint});
  }

  NodeId parse_type_constraint() {
    auto start_token = peek();
    NodeId lhs = parse_type_expression();
    Option<NodeId> rhs;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      rhs = parse_type_expression();
    }
    return m_output.add_node(start_token.loc, TypeConstraintNode{lhs, rhs});
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
    if (is_identifier(next_token.type) && following_token.type == TokenType::COLON) {
      var = parse_identifier();
      ++m_token_index; // consume the ':' token
    }
    NodeId exc_type = parse_type_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_statement();
    return m_output.add_node(catch_token.loc, CatchClauseNode{exc_type, var, body});
  }

  NodeId parse_switch_statement() {
    auto switch_token = next();
    read_left_paren("Expected '(' after 'switch'");
    auto introductory_decls = parse_introductory_decls();
    NodeId expr = parse_expression();
    enforce_separator_after_introductory_decls(introductory_decls, expr);
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
        SwitchStatementNode{std::move(introductory_decls), expr, std::move(clauses), default_body}
    );
  }

  NodeId parse_switch_statement_case_clause() {
    auto case_token = next();
    NodeId header = parse_case_clause_header();
    NodeId body = parse_statement();
    return m_output.add_node(case_token.loc, CaseClauseNode{header, body});
  }

  NodeId parse_type_declaration() {
    auto type_token = next();
    auto name = parse_expression();
    read_token_type(TokenType::ASSIGN, "Expected '=' after type name in type declaration");
    NodeId type_expr = parse_type_expression();
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
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(fun_token.loc, FunctionDeclarationNode{name, signature, body});
  }

  Option<NodeId> try_parse_function_body() {
    auto start_token = peek();
    if (start_token.type != TokenType::LEFT_BRACE && start_token.type != TokenType::ASSIGN) {
      return None();
    }
    ++m_token_index; // consume the '{' or '=' token

    Option<NodeId> expression;
    Option<List<NodeId>> stmts;
    bool is_default = false;
    bool is_deleted = false;

    if (start_token.type == TokenType::ASSIGN) {
      auto next_token = peek();
      if (next_token.type == TokenType::KEYWORD_DEFAULT) {
        is_default = true;
        ++m_token_index; // consume the 'default' keyword
      } else if (next_token.type == TokenType::KEYWORD_DELETE) {
        is_deleted = true;
        ++m_token_index; // consume the 'delete' keyword
      } else {
        expression = parse_expression();
      }
    } else {
      stmts = List<NodeId>();
      parse_statements(stmts.value(), TokenType::RIGHT_BRACE);
      ++m_token_index; // consume the '}' token
    }

    return m_output.add_node(
        start_token.loc, FunctionBodyNode{expression, std::move(stmts), is_default, is_deleted}
    );
  }

  NodeId parse_function_signature() {
    auto start_position = peek().loc;

    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    List<NodeId> parameters = parse_function_parameter_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    Option<NodeId> capture_list = try_parse_function_signature_capture_annotation_list();
    Option<NodeId> return_type = try_parse_function_return_type();

    return m_output.add_node(
        start_position,
        FunctionSignatureNode{
            generic_parameter_list,
            std::move(parameters),
            implicit_parameter_list,
            capture_list,
            return_type
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
      type = parse_type_expression();
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
          next_token.loc, ImplicitParameterListNode{std::move(implicit_parameters)}
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
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in while statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        while_token.loc, WhileStatementNode{std::move(introductory_decls), condition, body}
    );
  }

  NodeId parse_for_in_statement() {
    auto for_token = next();
    read_left_paren("Expected '(' after 'for' keyword in for-in statement");
    auto introductory_decls = parse_introductory_decls();
    List<NodeId> vars;
    vars.push_back(parse_expression());
    enforce_separator_after_introductory_decls(introductory_decls, vars[0]);
    while (peek().type == TokenType::COMMA) {
      ++m_token_index; // consume the ',' token
      vars.push_back(parse_expression());
    }
    read_token_type(TokenType::KEYWORD_IN, "Expected 'in' keyword in for-in statement");
    NodeId iterable = parse_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after iterable in for-in statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        for_token.loc,
        ForInStatementNode{std::move(introductory_decls), std::move(vars), iterable, body}
    );
  }

  NodeId parse_throw_statement() {
    auto throw_token = next();
    Option<NodeId> expr;
    auto next_token = peek();
    if (next_token.loc.line == throw_token.loc.line && next_token.type != TokenType::SEMICOLON &&
        next_token.type != TokenType::END_OF_FILE && next_token.type != TokenType::RIGHT_BRACE) {
      expr = parse_expression();
    }
    return m_output.add_node(throw_token.loc, ThrowStatementNode{expr});
  }

  NodeId parse_if_statement() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if' keyword in if statement");
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in if statement");
    NodeId then_branch = parse_statement();
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' keyword
      else_branch = parse_statement();
    }
    return m_output.add_node(
        if_token.loc,
        IfStatementNode{std::move(introductory_decls), condition, then_branch, else_branch}
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

  NodeId parse_const_declaration() {
    auto const_token = next();
    NodeId target = parse_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_type_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = parse_expression();
    }
    return m_output.add_node(
        const_token.loc, ConstDeclarationNode{target, type_annotation, expression}
    );
  }

  NodeId parse_let_declaration() {
    auto let_token = next();
    NodeId target = parse_expression();
    Option<NodeId> type_annotation;
    Option<NodeId> expression;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_type_expression();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expression = parse_expression();
    }
    return m_output.add_node(
        let_token.loc, LetDeclarationNode{target, type_annotation, expression}
    );
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
    NodeId left = parse_descend_expr_await_ref_copy_move();
    auto next_token = peek();
    while (next_token.type == TokenType::STAR || next_token.type == TokenType::SLASH ||
           next_token.type == TokenType::PERCENT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_await_ref_copy_move();
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

  NodeId parse_descend_expr_await_ref_copy_move() {
    auto next_token = peek();
    auto start_location = next_token.loc;
    if (next_token.type == TokenType::KEYWORD_AWAIT) {
      ++m_token_index; // consume the 'await' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move();
      return m_output.add_node(start_location, AwaitExpressionNode{expr});
    } else if (next_token.type == TokenType::KEYWORD_MOVE) {
      ++m_token_index; // consume the 'move' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move();
      return m_output.add_node(start_location, MoveExpressionNode{expr});
    } else if (next_token.type == TokenType::KEYWORD_COPY) {
      ++m_token_index; // consume the 'copy' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move();
      return m_output.add_node(start_location, CopyExpressionNode{expr});
    } else if (next_token.type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_descend_expr_await_ref_copy_move();
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
    auto left = parse_descend_expr_impl();
    auto next_token = peek();
    while (next_token.type == TokenType::DOT_NO_W || next_token.type == TokenType::NUMBER_FIELD ||
           next_token.type == TokenType::LEFT_BRACKET_NO_W ||
           next_token.type == TokenType::LEFT_PAREN_NO_W) {
      if (next_token.type == TokenType::DOT_NO_W) {
        ++m_token_index; // consume the '.' operator
        auto next_type = peek().type;
        if (!is_identifier_no_w(next_type) && next_type != TokenType::KEYWORD_OPERATOR) {
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

  NodeId parse_type_expression() {
    auto next_token = peek();
    auto start_location = next_token.loc;
    NodeId left;
    if (next_token.type == TokenType::STAR) {
      ++m_token_index; // consume the '*' operator
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_type_expression();
      left = m_output.add_node(start_location, DerefExpressionNode{is_const, expr});
    } else if (next_token.type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_type_expression();
      left = m_output.add_node(start_location, RefExpressionNode{is_const, expr});
    } else {
      left = parse_descend_expr_impl();
    }
    while (peek().type == TokenType::LEFT_BRACKET_NO_W) {
      ++m_token_index; // consume the '[' operator
      NodeId index_expr = parse_expression();
      read_token_type(
          TokenType::RIGHT_BRACKET, "Expected ']' after index expression in index access"
      );
      left = m_output.add_node(start_location, IndexingExpressionNode{left, index_expr});
    }
    return left;
  }

  NodeId parse_descend_expr_impl() {
    auto start_token = peek();
    if (start_token.type == TokenType::KEYWORD_IMPL) {
      ++m_token_index; // consume the 'impl' keyword
      NodeId type_expr = parse_type_expression();
      return m_output.add_node(start_token.loc, ImplTypeExpressionNode{type_expr});
    }
    return parse_descend_expr_scope_resolution();
  }

  NodeId parse_descend_expr_scope_resolution() {
    auto start_location = peek().loc;
    NodeId left = parse_atom();
    while (peek().type == TokenType::DOUBLE_COLON_NO_W) {
      ++m_token_index; // consume the '::' operator
      auto next_type = peek().type;
      if (!is_identifier_no_w(next_type) && next_type != TokenType::KEYWORD_OPERATOR) {
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
    switch (peek().type) {
    case TokenType::IDENTIFIER:
    case TokenType::IDENTIFIER_NO_W:
    case TokenType::QUOTED_IDENTIFIER:
    case TokenType::QUOTED_IDENTIFIER_NO_W:
    case TokenType::KEYWORD_OPERATOR:
      if (is_start_of_lambda_expression()) {
        return parse_lambda_expression();
      }
      return parse_identifier();
    case TokenType::STRING_LITERAL:
      return parse_string_literal();
    case TokenType::NUMBER:
    case TokenType::NUMBER_FIELD:
      return parse_number_literal();
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_PAREN_NO_W:
      if (is_start_of_lambda_expression()) {
        return parse_lambda_expression();
      }
      return parse_parenthesized_expression();
    case TokenType::LEFT_BRACKET:
    case TokenType::LEFT_BRACKET_NO_W:
      return parse_bracket_expression();
    case TokenType::LEFT_BRACE:
      return parse_brace_expression();
    case TokenType::KEYWORD_IF:
      return parse_if_expression();
    case TokenType::KEYWORD_TRY:
      return parse_try_expression();
    case TokenType::KEYWORD_SWITCH:
      return parse_switch_expression();
    case TokenType::KEYWORD_FUN:
      return parse_function_expression();
    case TokenType::KEYWORD_BOOL:
      return parse_bool_type();
    case TokenType::KEYWORD_TRUE:
    case TokenType::KEYWORD_FALSE:
      return parse_boolean_literal();
    case TokenType::KEYWORD_THIS:
      return parse_this_literal();
    default:
      String err("Expected expression, got token ");
      m_token_formatter.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
    }
  }

  NodeId parse_this_literal() {
    auto this_token = next();
    return m_output.add_node(this_token.loc, ThisLiteralNode{});
  }

  NodeId parse_boolean_literal() {
    auto bool_token = next();
    return m_output.add_node(
        bool_token.loc, BooleanLiteralNode{bool_token.type == TokenType::KEYWORD_TRUE}
    );
  }

  NodeId parse_bool_type() {
    auto bool_token = next();
    return m_output.add_node(bool_token.loc, BoolTypeNode{});
  }

  NodeId parse_function_expression() {
    auto fun_token = next();
    NodeId signature = parse_function_signature();
    if (peek().type != TokenType::LEFT_BRACE) {
      throw_parser_error_at_current_location(
          "Expected '{' to begin function body in function expression"
      );
    }
    NodeId body = try_parse_function_body().value();
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
      if (peek().type == TokenType::ASSIGN) {
        ++m_token_index; // consume the '=' token
        operator_node = m_output.add_node(start_location, OperatorIdentIxAssignNode{});
      } else {
        operator_node = m_output.add_node(start_location, OperatorIdentIxNode{});
      }
      break;
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_PAREN_NO_W:
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' following 'operator('");
      operator_node = m_output.add_node(start_location, OperatorIdentFuncallNode{});
      break;
    case TokenType::KEYWORD_AS: {
      auto type = parse_type_expression();
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
    auto introductory_decls = parse_introductory_decls();
    NodeId expr = parse_expression();
    enforce_separator_after_introductory_decls(introductory_decls, expr);
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
        SwitchExpressionNode{std::move(introductory_decls), expr, std::move(clauses), default_body}
    );
  }

  NodeId parse_switch_expression_case_clause() {
    auto case_token = next();
    NodeId header = parse_case_clause_header();
    NodeId body = parse_expression();
    return m_output.add_node(case_token.loc, CaseClauseNode{header, body});
  }

  NodeId parse_case_clause_header() {
    auto start_token = peek();
    Option<List<NodeId>> introductory_decls;
    Option<List<NodeId>> exprs;
    Option<NodeId> when_clause;

    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the left paren
      introductory_decls = parse_introductory_decls();
      exprs = List<NodeId>();
      do {
        exprs.value().push_back(parse_expression());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the comma
        }
      } while (peek().type != TokenType::RIGHT_PAREN);
      enforce_separator_after_introductory_decls(introductory_decls.value(), exprs.value()[0]);
      ++m_token_index; // consume the right paren
    }

    if (peek().type == TokenType::KEYWORD_WHEN) {
      ++m_token_index; // consume the 'when' keyword
      read_left_paren("Expected '(' after 'when' in case clause");
      when_clause = parse_expression();
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'when' clause");
    }

    if (!introductory_decls.has_value() && !exprs.has_value() && !when_clause.has_value()) {
      throw_parser_error(start_token.id, "Expected case clause header");
    }

    return m_output.add_node(
        start_token.loc,
        CaseClauseHeaderNode{std::move(introductory_decls), std::move(exprs), when_clause}
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
    if (is_identifier(peek().type) && peek(1).type == TokenType::COLON) {
      var = parse_identifier();
      ++m_token_index; // consume the ':' token
    }
    NodeId exc_type = parse_type_expression();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_expression();
    return m_output.add_node(catch_token.loc, CatchClauseNode{exc_type, var, body});
  }

  NodeId parse_if_expression() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if'");
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expression();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' expression");
    NodeId then_branch = parse_expression();
    read_token_type(
        TokenType::KEYWORD_ELSE, "Expected 'else' after then-branch of 'if' expression"
    );
    NodeId else_branch = parse_expression();
    return m_output.add_node(
        if_token.loc,
        IfExpressionNode{std::move(introductory_decls), condition, then_branch, else_branch}
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
    if (next_token.type == TokenType::END_OF_FILE) {
      throw_parser_error_at_current_location("Unexpected end of file after '{'. Expected object "
                                             "literal, object type, or block expression.");
    }
    if (next_token.type == TokenType::RIGHT_BRACE || next_token.type == TokenType::DOT) {
      return parse_object_literal();
    } else if (is_identifier(next_token.type) && peek(2).type == TokenType::COLON) {
      return parse_object_type();
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
    return m_output.add_node(open_brace.loc, AnonymousStructLiteralNode{std::move(entries)});
  }

  NodeId parse_object_type() {
    auto open_brace = next();
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      auto field_token = peek();
      auto field = expect_identifier("Expected field name in object literal");
      read_token_type(TokenType::COLON, "Expected ':' after field name in object type");
      NodeId type = parse_type_expression();
      entries.push_back(m_output.add_node(field_token.loc, KeyValueEntryNode{field, type}));
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    }
    ++m_token_index; // consume the right brace
    return m_output.add_node(open_brace.loc, AnonymousStructTypeNode{std::move(entries)});
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
    if (!is_identifier(token.type) && token.type != TokenType::KEYWORD_OPERATOR) {
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
    if (is_identifier(next_token.type)) {
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

        if (is_identifier(next_token.type)) {
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
    if (is_identifier(next_token.type) && peek(1).type == TokenType::ASSIGN) {
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
