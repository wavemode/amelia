#pragma once

#include <cstddef>

#include "lexer/data/token.hpp"
#include "lexer/interface/token_repository.hpp"
#include "util/data/list.hpp"

namespace amelia {

struct LexerResult : public ITokenRepository {
  ConstSlice<Token> tokens() const;

  const Token &get_token(TokenId token_id) const override;

  TokenId add_token(Token token);

  Text token_text_slice(TokenId start, TokenId end) const;

private:
  List<Token> m_tokens;
};

} // namespace amelia
