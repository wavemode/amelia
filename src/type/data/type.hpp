#pragma once

#include <cstdint>

#include "operator/data/binary_operator_kind.hpp"
#include "operator/data/unary_operator_kind.hpp"
#include "util/data/flex.hpp"
#include "util/data/utility.hpp"

namespace amelia {

class Integer;
class Serialize;
struct Expression;
template <typename T> class Option;
using NodeId = int32_t;

struct Type : FlexFromThis<Type>, Dynamic {

  /// Conversions

public:
  virtual bool is_resolved() const;
  virtual bool is_comptime_const() const;

protected:
  virtual Flex<Type> resolve() const;
  virtual Flex<Type> remove_comptime_const() const;
  virtual bool unify(const Type &assignment_type) const;
  virtual Option<Flex<Expression>> coerce(const Type &assignment_type, const Expression &expr)
      const;
  virtual Option<Flex<Expression>> cast(const Type &assignment_type, const Expression &expr) const;

  /// Conversion Helpers

public:
  Flex<Type> resolve_type() const;
  Flex<Type> remove_comptime_const_from_type() const;
  bool unify_type(const Type &assignment_type) const;
  Option<Flex<Expression>> coerce_expr(const Expression &expr) const;
  Option<Flex<Expression>> coerce_expr(const Type &expr_type, const Expression &expr) const;
  Option<Flex<Expression>> cast_expr(const Expression &expr) const;
  Option<Flex<Expression>> cast_expr(const Type &expr_type, const Expression &expr) const;

  /// Numerics

  virtual bool is_floating_point() const;
  virtual bool is_integral() const;
  virtual bool has_native_numeric_repr() const;
  virtual uint64_t repr_bit_size() const;
  virtual Integer min_value() const;
  virtual Integer max_value() const;
  bool can_represent_range(const Integer &min, const Integer &max) const;

  /// Operators

  virtual Option<Flex<Expression>> perform_binary_op(
      NodeId expr_node_id,
      BinaryOperatorKind op_kind,
      const Expression &left_expr,
      const Type &right_type,
      const Expression &right_expr
  ) const;

  virtual Option<Flex<Expression>> perform_unary_op(
      NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
  ) const;

  /// Traits

  virtual bool is_trivial() const;
  virtual bool is_primitive() const;

  /// Utilities

  virtual Serialize serialize() const = 0;

  /// System

  virtual ~Type() = default;
};

} // namespace amelia
