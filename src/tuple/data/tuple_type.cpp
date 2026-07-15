#include "tuple_type.hpp"

#include "expr/data/expression.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/list.hpp"
#include "util/data/serialize.hpp"
#include "util/data/slice.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool TupleType::is_resolved() const {
  if (is_resolved_cached.has_value()) {
    return is_resolved_cached.value();
  }

  for (const auto &element_type : element_types) {
    if (!element_type->is_resolved()) {
      is_resolved_cached = false;
      return false;
    }
  }

  is_resolved_cached = true;
  return true;
}

Flex<Type> TupleType::resolve() const {
  if (resolved_type_cached.has_value()) {
    return resolved_type_cached.value();
  }

  auto resolved_tuple = emplace_flex<TupleType>();
  for (const auto &element_type : element_types) {
    resolved_tuple->element_types.push_back(element_type->resolve());
  }

  resolved_type_cached = resolved_tuple;
  return resolved_tuple;
}

bool TupleType::is_comptime_const() const {
  if (has_comptime_const_cached.has_value()) {
    return has_comptime_const_cached.value();
  }

  for (const auto &element_type : element_types) {
    if (!element_type->is_comptime_const()) {
      has_comptime_const_cached = false;
      return false;
    }
  }

  has_comptime_const_cached = true;
  return true;
}

Flex<Type> TupleType::remove_comptime_const() const {
  if (remove_comptime_const_cached.has_value()) {
    return remove_comptime_const_cached.value();
  }

  auto new_tuple = emplace_flex<TupleType>();
  for (const auto &element_type : element_types) {
    new_tuple->element_types.push_back(element_type->remove_comptime_const());
  }

  remove_comptime_const_cached = new_tuple;
  return new_tuple;
}

bool TupleType::unify(const Type &assignment_type) const {
  if (!assignment_type.is<TupleType>()) {
    return false;
  }

  const auto &assignment_tuple = assignment_type.as<TupleType>();
  if (element_types.size() != assignment_tuple.element_types.size()) {
    return false;
  }

  for (size_t i = 0; i < element_types.size(); ++i) {
    if (!element_types[i]->unify(*assignment_tuple.element_types[i])) {
      return false;
    }
  }

  return true;
}

Option<Flex<Expression>> TupleType::coerce(const Type &assignment_type, const Expression &expr)
    const {
  return None(); // TODO: compatible tuples
}

Serialize TupleType::serialize() const {
  String repr("(");
  for (size_t i = 0; i < element_types.size(); ++i) {
    if (i > 0) {
      repr.append(", ");
    }
    element_types[i]->serialize().to_string(repr);
  }
  repr.append(')');
  return Serialize::literal(repr);
}

} // namespace amelia
