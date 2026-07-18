#include "goto_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize GotoStatement::serialize() const {
  Serialize result;
  result.set_object_name("GotoStatement");
  result.add_object_field("label", Serialize::literal(String(label)));
  return result;
}

} // namespace amelia
