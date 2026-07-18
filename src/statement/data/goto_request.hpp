#pragma once

#include <cstdint>

namespace amelia {

struct Expression;

struct GotoRequest {
  const Expression *goto_stmt;
  uint32_t goto_scope_level;
};

} // namespace amelia
