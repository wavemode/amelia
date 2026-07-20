#include "continue_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize ContinueStatement::serialize() const {
  return Serialize::literal("ContinueStatement()");
}

} // namespace amelia
