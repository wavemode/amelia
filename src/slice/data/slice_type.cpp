#include "slice_type.hpp"

#include "type/logic/type_conversion.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool SliceType::is_resolved() const {
  return element_type->is_resolved();
}

Flex<Type> SliceType::resolve() const {
  auto resolved_slice = emplace_flex<SliceType>();
  resolved_slice->element_type = element_type->resolve();
  return resolved_slice;
}

bool SliceType::is_comptime_const() const {
  return element_type->is_comptime_const();
}

Flex<Type> SliceType::remove_comptime_const() const {
  auto result = emplace_flex<SliceType>();
  result->element_type = element_type->remove_comptime_const();
  return result;
}

bool SliceType::unify(const Type &assignment_type) const {
  if (!assignment_type.is<SliceType>()) {
    return false;
  }
  const auto &assignment_slice = assignment_type.as<SliceType>();
  return Type::unify_types(element_type, assignment_slice.element_type);
}

Serialize SliceType::serialize() const {
  String repr("[");
  element_type->serialize().to_string(repr);
  repr.append(']');
  return Serialize::literal(repr);
}

} // namespace amelia
