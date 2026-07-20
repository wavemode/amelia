#include "break_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize BreakStatement::serialize() const {
  return Serialize::literal("BreakStatement()");
}

} // namespace amelia
