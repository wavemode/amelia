#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token_formatter.h"
#include "data/parser/abstract_node_repository.h"

namespace amelia {

class NodeFormatter {
public:
  NodeFormatter(const AbstractNodeRepository &node_repo, const AbstractTokenRepository &token_repo)
      : m_token_formatter(TokenFormatter(token_repo)), m_node_repo(node_repo) {}

  void format_node(AbstractString &out, size_t node_id) const;

  const TokenFormatter m_token_formatter;
  const AbstractNodeRepository &m_node_repo;
};

} // namespace amelia
