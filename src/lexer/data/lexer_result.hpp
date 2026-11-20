#pragma once

#include "lexer/data/token.hpp"
#include "lexer/interface/token_repository.hpp"
#include "lexer_context.hpp"
#include "util/data/list.hpp"

namespace amelia {

struct LexerResult : public ITokenRepository {
  LexerResult() noexcept;
  LexerResult(LexerContext ctx) noexcept;

  ConstSlice<Token> tokens() const;

  const FilePath &filename() const;
  void set_filename(FilePath &&filename);

  const Token &get_token(TokenId token_id) const override;

  TokenId add_token(Token token);

  Text token_text_slice(TokenId start, TokenId end) const;

private:
  List<Token> m_tokens;
  LexerContext m_ctx;
};

} // namespace amelia
