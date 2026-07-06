#pragma once

#include <cstddef>

#include "lexer/data/abstract_token_repository.hpp"

namespace amelia {

struct AbstractString;

class TokenFormatter {
public:
  TokenFormatter(const AbstractTokenRepository &repo);

  void format_token(AbstractString &out, size_t token_id) const;

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
