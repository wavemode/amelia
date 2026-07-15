#include "pointer_type.hpp"

#include "expr/data/expression.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool PointerType::is_resolved() const {
  return pointee->is_resolved();
}

Flex<Type> PointerType::resolve() const {
  auto resolved_ref = emplace_flex<PointerType>();
  resolved_ref->pointee = pointee->resolve();
  resolved_ref->is_const = is_const;
  return resolved_ref;
}

bool PointerType::is_comptime_const() const {
  return pointee->is_comptime_const();
}

Flex<Type> PointerType::remove_comptime_const() const {
  auto result = emplace_flex<PointerType>();
  result->pointee = pointee->remove_comptime_const();
  result->is_const = is_const;
  return result;
}

bool PointerType::unify(const Type &assignment_type) const {
  if (!assignment_type.is<PointerType>()) {
    return false;
  }
  auto &assignment_type_ref = assignment_type.as<PointerType>();
  if (!Type::unify_types(pointee, assignment_type_ref.pointee)) {
    return false;
  }
  return is_const == assignment_type_ref.is_const;
}

Option<Flex<Expression>> PointerType::coerce(const Type &assignment_type, const Expression &expr)
    const {
  if (assignment_type.is<PointerType>()) {
    auto &expr_ref_type = assignment_type.as<PointerType>();
    if (
          // If target is not const, assignment must also not be const
          (is_const || !expr_ref_type.is_const)
      ) {
      if (
            // Pointers refer to the same type
            // TODO: compatible types
            Type::unify_types(pointee, expr_ref_type.pointee)
        ) {
        return native_type_cast(*this, expr);
      }
    }
  }
  return None();
}

Serialize PointerType::serialize() const {
  String repr("*");
  if (is_const) {
    repr.append("const ");
  }
  pointee->serialize().to_string(repr);
  return Serialize::literal(repr);
}

} // namespace amelia
