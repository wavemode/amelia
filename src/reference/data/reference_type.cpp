#include "reference_type.hpp"

#include "array/data/array_type.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_string_type.hpp"
#include "expr/data/expression.hpp"
#include "slice/data/slice_type.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"

namespace amelia {

bool ReferenceType::is_resolved() const {
  return referent->is_resolved();
}

Flex<Type> ReferenceType::resolve() const {
  auto resolved_ref = emplace_flex<ReferenceType>();
  resolved_ref->referent = referent->resolve();
  resolved_ref->is_const = is_const;
  resolved_ref->is_move = is_move;
  return resolved_ref;
}

bool ReferenceType::is_comptime_const() const {
  return referent->is_comptime_const();
}

Flex<Type> ReferenceType::remove_comptime_const() const {
  auto result = emplace_flex<ReferenceType>();
  result->referent = referent->remove_comptime_const();
  result->is_const = is_const;
  result->is_move = is_move;
  return result;
}

bool ReferenceType::unify(const Type &assignment_type) const {
  if (assignment_type.is<ConstStringType>()) {
    return unify(STR_REF_TYPE);
  }

  if (!assignment_type.is<ReferenceType>()) {
    return false;
  }
  auto &assignment_type_ref = assignment_type.as<ReferenceType>();
  if (!Type::unify_types(referent, assignment_type_ref.referent)) {
    return false;
  }
  return is_const == assignment_type_ref.is_const && is_move == assignment_type_ref.is_move;
}

Option<Flex<Expression>> ReferenceType::coerce(const Type &assignment_type, const Expression &expr)
    const {
  if (assignment_type.is<ReferenceType>()) {
    auto &expr_ref_type = assignment_type.as<ReferenceType>();
    if (
          // If target is not const, assignment must also not be const
          (is_const || !expr_ref_type.is_const) &&

          // If target type is non-trivial, "move" quality of references must be the same
          (referent->is_trivial() ||
           (is_move == expr_ref_type.is_move))
      ) {
      if (
            // References refer to the same type
            // TODO: compatible types
            Type::unify_types(referent, expr_ref_type.referent)
        ) {
        return native_type_cast(*this, expr);
      } else if (
            // Target type refers to a slice and expr type refers to an array of the same type
            referent->is<SliceType>() &&
            expr_ref_type.referent->is<ArrayType>() &&
            Type::unify_types(referent->as<SliceType>().element_type,
              expr_ref_type.referent->as<ArrayType>().element_type
            )
        ) {
        return native_type_cast(*this, expr);
      }
    }
  } else if (assignment_type.is<ConstStringType>()) {
    // If the assignment type is a const string, we can coerce to &str
    if (referent->is<BuiltinType>() &&
        referent->as<BuiltinType>().builtin_kind == BuiltinKind::Str) {
      return native_type_cast(*this, expr);
    }
  }
  return None();
}

Serialize ReferenceType::serialize() const {
  String repr("&");
  if (is_const) {
    repr.append("const ");
  }
  if (is_move) {
    repr.append("move ");
  }
  referent->serialize().to_string(repr);
  return Serialize::literal(repr);
}

} // namespace amelia
