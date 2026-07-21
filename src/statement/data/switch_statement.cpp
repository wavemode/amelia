#include "switch_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize SwitchStatement::serialize() const {
  Serialize result;
  result.set_object_name("SwitchStatement");
  if (case_clauses.size() > 0) {
    Serialize case_clauses_list;
    for (const auto &case_clause : case_clauses) {
      case_clauses_list.add_list_item(case_clause->serialize());
    }
    result.add_object_field("case_clauses", case_clauses_list);
  }
  if (default_body.has_value()) {
    result.add_object_field("default_body", default_body.value()->serialize());
  }
  return result;
}

} // namespace amelia
