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
    while (peek().type != TokenType::END_OF_FILE) {
      if (peek().type == TokenType::SEMICOLON) {
        ++m_token_index;
        continue;
      }
      top_level_statements.push_back(parse_top_level_statement());
      if (top_level_statements.size() > 1) {
        auto prev_stmt_info = m_output.get_node_info(
            top_level_statements[top_level_statements.size() - 2]
        );
        auto curr_stmt_info = m_output.get_node_info(
            top_level_statements[top_level_statements.size() - 1]
        );
        if (prev_stmt_info.location.line == curr_stmt_info.location.line) {
          throw_parser_error_at_current_location(
              "Multiple statements on the same line are not allowed"
          );
        }
      }
    }
    return m_output.add_ModuleNode(start_token.loc, ModuleNode{std::move(top_level_statements)});
  }

  NodeId parse_top_level_statement() {
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
    auto const_token = next();
    NodeId target = parse_expression();
    auto assign_token = read_token_type(
        TokenType::ASSIGN, "Expected '=' after identifier in const statement"
    );
    NodeId expression = parse_expression();
    return m_output.add_ConstStatementNode(const_token.loc, ConstStatementNode{target, expression});
  }

  NodeId parse_let_statement() {
    auto let_token = next();
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
    } else {
      String err("Expected expression, got token ");
      m_input.format_token(err, m_token_index);
      throw_parser_error_at_current_location(std::move(err));
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
