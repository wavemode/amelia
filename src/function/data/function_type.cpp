#include "function_type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "source/data/char_literal.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/integer.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize FunctionType::serialize() const {
  Serialize result;
  Serialize signatures_list;
  for (const auto &signature : signatures) {
    signatures_list.add_list_item(serialize_function_signature(signature));
  }
  result.set_object_name("FunctionType");
  result.add_object_field("name", Serialize::literal(name));
  result.add_object_field("signatures", move(signatures_list));
  return result;
}

} // namespace amelia
