#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token.h"
#include "data/source/number_literal.h"
#include "data/util/text_utils.h"
#include "prelude.h"

namespace amelia {

class TokenFormatter {
public:
  TokenFormatter(const AbstractTokenRepository &repo);

  void format_token(AbstractString &out, size_t token_id) const;

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
