#include "label_statement.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize LabelStatement::serialize() const {
  Serialize result;
  result.set_object_name("LabelStatement");
  result.add_object_field("name", Serialize::literal(String(name)));
  return result;
}

} // namespace amelia
