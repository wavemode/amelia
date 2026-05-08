#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token.h"
#include "data/source/number_literal.h"
#include "data/source/string_literal.h"
#include "prelude.h"

namespace amelia {

struct LexerResult : public AbstractTokenRepository {
  ConstSlice<Token> tokens() const {
    return m_tokens.data();
  }

  Text string_literal_contents(size_t token_id) const override {
    auto lit = get_string_literal(token_id);
    if (lit.buffer_end > m_string_literal_buffer.size() || lit.buffer_start > lit.buffer_end) {
      throw RuntimeError("Invalid string literal buffer start and end");
    }
    return Text(ConstSlice(
        m_string_literal_buffer.data().ptr() + lit.buffer_start, lit.buffer_end - lit.buffer_start
    ));
  }

  NumberLiteral get_number_literal(size_t token_id) const override {
    auto result = m_number_literals.find(token_id);
    if (!result.has_value()) {
      throw RuntimeError("Token does not have a number literal");
    }
    return result.value();
  }

  Token get_token(size_t token_id) const override {
    if (token_id >= m_tokens.size()) {
      throw RuntimeError("Invalid token ID");
    }
    return m_tokens[token_id];
  }

  size_t add_token(Token token) {
    size_t token_id = m_tokens.size();
    m_tokens.push_back(token);
    return token_id;
  }

  void add_string_literal(size_t token_id, StringLiteral lit) {
    m_string_literals.set(token_id, lit);
  }

  void add_number_literal(size_t token_id, NumberLiteral lit) {
    m_number_literals.set(token_id, lit);
  }

  size_t string_literal_buffer_size() const {
    return m_string_literal_buffer.size();
  }

  void append_byte_to_string_literal_buffer(char byte) {
    m_string_literal_buffer.push_back(byte);
  }

  void append_code_point_to_string_literal_buffer(uint32_t code_point) {
    CharIterator::append(m_string_literal_buffer, code_point);
  }

private:
  StringLiteral get_string_literal(size_t token_id) const {
    auto result = m_string_literals.find(token_id);
    if (!result.has_value()) {
      throw RuntimeError("Token does not have a string literal");
    }
    return result.value();
  }

  List<Token> m_tokens;
  List<char> m_string_literal_buffer;
  Map<size_t, StringLiteral> m_string_literals;
  Map<size_t, NumberLiteral> m_number_literals;
};

} // namespace amelia
