#include "return_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize ReturnStatement::serialize() const {
  Serialize result;
  result.set_object_name("ReturnStatement");
  if (value.has_value()) {
    result.add_object_field("value", value.value()->serialize());
  }
  return result;
}

} // namespace amelia
