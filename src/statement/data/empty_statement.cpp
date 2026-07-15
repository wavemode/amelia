#include "empty_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize EmptyStatement::serialize() const {
  return Serialize::literal("EmptyStatement()");
}

} // namespace amelia
