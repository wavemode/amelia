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
      stmts.push_back(parse_statement());
      if (stmts.size() > 1) {
        auto prev_stmt_info = m_output.get_node_info(stmts[stmts.size() - 2]);
        auto curr_stmt_info = m_output.get_node_info(stmts[stmts.size() - 1]);
        if (prev_stmt_info.location.line == curr_stmt_info.location.line) {
          throw_parser_error_at_current_location(
              "Multiple statements on the same line are not allowed"
          );
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
      String err("Expected statement, got token ");
      m_input.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
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

  NodeId parse_expression() {
    return parse_atom();
  }

  NodeId parse_atom() {
    auto token = peek();
    if (token.type == TokenType::IDENTIFIER) {
      return parse_identifier();
    } else if (token.type == TokenType::STRING_LITERAL) {
      return parse_string_literal();
    } else if (token.type == TokenType::NUMBER) {
      return parse_number_literal();
    } else if (token.type == TokenType::LEFT_PAREN || token.type == TokenType::FUNCALL_START) {
      return parse_parenthesized_expression();
    } else if (token.type == TokenType::LEFT_BRACKET || token.type == TokenType::IX_START) {
      return parse_array_literal();
    } else if (token.type == TokenType::LEFT_BRACE) {
      return parse_brace_expression();
    } else {
      String err("Expected expression, got token ");
      m_input.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
    }
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
    if (next_token.type == TokenType::RIGHT_BRACE ||
        next_token.type == TokenType::DOTTED_IDENTIFIER) {
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
    List<KeyValueEntryNode> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      auto field_token = read_token_type(
          TokenType::DOTTED_IDENTIFIER, "Expected field name in object literal"
      );
      read_token_type(TokenType::ASSIGN, "Expected '=' after field name in object literal");
      NodeId value = parse_expression();
      entries.push_back(KeyValueEntryNode{field_token.id, value});
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
  TokenWithId read_token_type(TokenType expected, String error_message) {
    auto token = peek();
    if (token.type != expected) {
      throw_parser_error(token.id, std::move(error_message));
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
