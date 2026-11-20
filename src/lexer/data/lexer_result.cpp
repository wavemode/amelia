#include "lexer_result.hpp"

#include "util/data/text_utils.hpp"

namespace amelia {

LexerResult::LexerResult() noexcept = default;

LexerResult::LexerResult(LexerContext ctx) noexcept : m_ctx(move(ctx)) {}

ConstSlice<Token> LexerResult::tokens() const { return m_tokens.data(); }

const FilePath &LexerResult::filename() const { return m_ctx.filename; }

void LexerResult::set_filename(FilePath &&filename) {
  m_ctx.filename = move(filename);
}

const Token &LexerResult::get_token(TokenId token_id) const {
  if (token_id >= static_cast<TokenId>(m_tokens.size())) {
    throw_runtime_error("Invalid token ID");
  }
  return m_tokens[token_id];
}

TokenId LexerResult::add_token(Token token) {
  TokenId token_id = m_tokens.size();
  m_tokens.push_back(token);
  return token_id;
}

Text LexerResult::token_text_slice(TokenId start, TokenId end) const {
  CharIterator start_iter = get_token(start).location.position;
  CharIterator end_iter = get_token(end - 1).contents.end();
  return TextUtils::substr(start_iter.text(), start_iter, end_iter);
}

} // namespace amelia
