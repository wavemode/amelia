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
  TokenFormatter(const AbstractTokenRepository &repo) : m_repo(repo) {}

  void format_token(AbstractString &out, size_t token_id) const {
    auto token = m_repo.get_token(token_id);
    token_type_to_string(out, token.type);
    out.append("(");
    out.append(token.contents);
    out.append(")");
  }

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
