#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.hpp"
#include "data/lexer/token.hpp"
#include "data/source/number_literal.hpp"
#include "data/util/text_utils.hpp"
#include "prelude.hpp"

namespace amelia {

class TokenFormatter {
public:
  TokenFormatter(const AbstractTokenRepository &repo);

  void format_token(AbstractString &out, size_t token_id) const;

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
