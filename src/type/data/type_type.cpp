#include "type_type.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool TypeType::internal_unify(const Type &assignment_type) const {
  return assignment_type.is<TypeType>() &&
         referenced_type->unify(assignment_type.as<TypeType>().referenced_type);
}

Serialize TypeType::serialize() const {
  String repr("Type[");
  referenced_type->serialize().to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

} // namespace amelia
