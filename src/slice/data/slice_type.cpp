#include "slice_type.hpp"

#include "array/data/array_type.hpp"
#include "expr/data/expression.hpp"
#include "reference/data/reference_type.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool SliceType::is_resolved() const {
  return element_type->is_resolved();
}

Flex<Type> SliceType::internal_resolve() const {
  auto resolved_slice = emplace_flex<SliceType>();
  resolved_slice->element_type = element_type->resolve();
  return resolved_slice;
}

bool SliceType::is_comptime_const() const {
  return element_type->is_comptime_const();
}

Flex<Type> SliceType::internal_remove_comptime_const() const {
  auto result = emplace_flex<SliceType>();
  result->element_type = element_type->remove_comptime_const();
  return result;
}

bool SliceType::internal_unify(const Type &assignment_type) const {
  if (!assignment_type.is<SliceType>()) {
    return false;
  }
  const auto &assignment_slice = assignment_type.as<SliceType>();
  return (is_const || !assignment_slice.is_const) &&
         element_type->unify(assignment_slice.element_type);
}

Option<Flex<Expression>> SliceType::internal_coerce(
    const Type &assignment_type, const Expression &expr
) const {
  if (assignment_type.is<ReferenceType>()) {
    auto &assignment_ref = assignment_type.as<ReferenceType>();
    if (assignment_ref.referent->is<ArrayType>()) {
      auto &assignment_array = assignment_ref.referent->as<ArrayType>();
      if (element_type->unify(assignment_array.element_type)) {
        return native_type_cast(*this, expr);
      }
    }
  }

  return None();
}

Serialize SliceType::serialize() const {
  String repr("[]");
  if (is_const) {
    repr.append("const ");
  }
  element_type->serialize().to_string(repr);
  return Serialize::literal(repr);
}

} // namespace amelia
