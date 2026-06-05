#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.hpp"
#include "data/lexer/token.hpp"
#include "prelude.hpp"

namespace amelia {

struct LexerResult : public AbstractTokenRepository {
  ConstSlice<Token> tokens() const {
    return m_tokens.data();
  }

  Token get_token(TokenId token_id) const override {
    if (token_id >= static_cast<TokenId>(m_tokens.size())) {
      throw RuntimeError("Invalid token ID");
    }
    return m_tokens[token_id];
  }

  TokenId add_token(Token token) {
    TokenId token_id = m_tokens.size();
    m_tokens.push_back(token);
    return token_id;
  }

  Text token_text_slice(TokenId start, TokenId end) const;

private:
  List<Token> m_tokens;
};

} // namespace amelia
