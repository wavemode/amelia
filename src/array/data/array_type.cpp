#include "array_type.hpp"

#include "expr/data/expression.hpp"

#include "type/logic/type_conversion.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

bool ArrayType::is_resolved() const {
  return element_type->is_resolved();
}

Flex<Type> ArrayType::resolve() const {
  auto resolved_array = emplace_flex<ArrayType>();
  resolved_array->element_type = element_type->resolve();
  resolved_array->size = size;
  return resolved_array;
}

bool ArrayType::is_comptime_const() const {
  return element_type->is_comptime_const();
}

Flex<Type> ArrayType::remove_comptime_const() const {
  auto result = emplace_flex<ArrayType>();
  result->element_type = element_type->remove_comptime_const();
  result->size = size;
  return result;
}

bool ArrayType::unify(const Type &assignment_type) const {
  if (!assignment_type.is<ArrayType>()) {
    return false;
  }

  const auto &assignment_array_type = assignment_type.as<ArrayType>();
  return size == assignment_array_type.size &&
         element_type->unify(assignment_array_type.element_type);
}

Option<Flex<Expression>> ArrayType::cast(const Type &assignment_type, const Expression &expr)
    const {
  if (!assignment_type.is<ArrayType>()) {
    return None();
  }

  const auto &assignment_array_type = assignment_type.as<ArrayType>();
  if (size > assignment_array_type.size) {
    return None();
  }

  if (!element_type->coerce(assignment_array_type.element_type, expr).has_value()) {
    return None();
  }

  return native_type_cast(assignment_array_type, expr);
}

Serialize ArrayType::serialize() const {
  return Serialize();
}

} // namespace amelia
