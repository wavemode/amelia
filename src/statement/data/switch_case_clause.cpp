#include "switch_case_clause.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize SwitchCaseClause::serialize() const {
  Serialize result;
  result.set_object_name("SwitchCaseClause");
  if (condition.has_value()) {
    result.add_object_field("condition", condition.value()->serialize());
  }
  if (expr_body.has_value()) {
    result.add_object_field("expr_body", expr_body.value()->serialize());
  }
  if (when_body.has_value()) {
    result.add_object_field("when_body", when_body.value()->serialize());
  }
  return result;
}

} // namespace amelia
