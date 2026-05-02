#pragma once

#include <cstddef>

#include "data/source/number_literal.h"
#include "data/util/list.h"
#include "data/util/map.h"
#include "data/util/option.h"
#include "data/util/ref.h"
#include "data/util/slice.h"
#include "data/util/string.h"
#include "data/util/text.h"
#include "token.h"

namespace amelia {

class Lexer;

class LexerResult {
public:
  Slice<const Token> tokens() const noexcept { return m_tokens; }

  Option<Text> string_literal(size_t token_id) const noexcept {
    return m_string_literals.find(token_id);
  }

  Option<NumberLiteral> number_literal(size_t token_id) const noexcept {
    return m_number_literals.find(token_id);
  }

  friend class Lexer;

private:
  List<Token> m_tokens;
  String m_string_literal_buffer;
  Map<size_t, Text> m_string_literals;
  Map<size_t, NumberLiteral> m_number_literals;
};

} // namespace amelia
