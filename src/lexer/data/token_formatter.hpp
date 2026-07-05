#pragma once

#include <cstddef>

#include "lexer/data/abstract_token_repository.hpp"
#include "lexer/data/token.hpp"
#include "prelude.hpp"
#include "source/data/number_literal.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

class TokenFormatter {
public:
  TokenFormatter(const AbstractTokenRepository &repo);

  void format_token(AbstractString &out, size_t token_id) const;

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
