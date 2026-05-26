#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token.h"
#include "prelude.h"

namespace amelia {

struct LexerResult : public AbstractTokenRepository {
  ConstSlice<Token> tokens() const {
    return m_tokens.data();
  }

  Token get_token(TokenId token_id) const override {
    if (token_id >= static_cast<TokenId>(m_tokens.size())) {
      throw std::runtime_error("Invalid token ID");
    }
    return m_tokens[token_id];
  }

  TokenId add_token(Token token) {
    TokenId token_id = m_tokens.size();
    m_tokens.push_back(token);
    return token_id;
  }

private:
  List<Token> m_tokens;
};

} // namespace amelia
