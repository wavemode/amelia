#include "lexer_result.hpp"

#include "util/data/text_utils.hpp"

namespace amelia {

Text LexerResult::token_text_slice(TokenId start, TokenId end) const {
  CharIterator start_iter = get_token(start).location.position;
  CharIterator end_iter = get_token(end - 1).contents.end();
  return TextUtils::substr(start_iter.text(), start_iter, end_iter);
}

} // namespace amelia
