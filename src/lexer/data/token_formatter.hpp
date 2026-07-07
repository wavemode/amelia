#pragma once

#include <cstddef>

#include "lexer/interface/token_repository.hpp"

namespace amelia {

struct AbstractString;

class TokenFormatter {
public:
  TokenFormatter(const ITokenRepository &repo);

  void format_token(AbstractString &out, size_t token_id) const;

private:
  const ITokenRepository &m_repo;
};

} // namespace amelia
