#include "if_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize IfStatement::serialize() const {
  Serialize result;
  result.set_object_name("IfStatement");
  result.add_object_field("condition", condition->serialize());
  result.add_object_field("then_branch", then_branch->serialize());
  if (else_branch.has_value()) {
    result.add_object_field("else_branch", else_branch.value()->serialize());
  }
  return result;
}

} // namespace amelia
