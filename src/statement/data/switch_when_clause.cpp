#include "switch_when_clause.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize SwitchWhenClause::serialize() const {
  Serialize result;
  result.set_object_name("SwitchWhenClause");
  result.add_object_field("condition", condition->serialize());
  result.add_object_field("body", body->serialize());
  return result;
}

} // namespace amelia
