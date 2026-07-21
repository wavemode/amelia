#pragma once

#include "builtin/data/builtin_kind.hpp"
#include "type/data/type.hpp"

namespace amelia {

struct BuiltinType : Type {
  BuiltinType(BuiltinKind);
  BuiltinKind builtin_kind;

  bool unify(const Type &assignment_type) const override;
  Option<Flex<Expression>> coerce(const Type &assignment_type, const Expression &expr)
      const override;
  Option<Flex<Expression>> cast(const Type &assignment_type, const Expression &expr) const override;

  bool is_integral() const override;
  bool is_floating_point() const override;
  bool has_native_numeric_repr() const override;
  uint64_t repr_bit_size() const override;
  Integer min_value() const override;
  Integer max_value() const override;

  Option<Flex<Expression>> perform_binary_op(
      NodeId expr_node_id,
      BinaryOperatorKind op_kind,
      const Expression &left_expr,
      const Type &right_type,
      const Expression &right_expr
  ) const override;

  Option<Flex<Expression>> perform_unary_op(
      NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
  ) const override;

  bool is_primitive() const override;

  Serialize serialize() const override;
};

extern Flex<Type> BYTE_TYPE;
extern Flex<Type> UBYTE_TYPE;
extern Flex<Type> SHORT_TYPE;
extern Flex<Type> USHORT_TYPE;
extern Flex<Type> INT_TYPE;
extern Flex<Type> UINT_TYPE;
extern Flex<Type> LONG_TYPE;
extern Flex<Type> ULONG_TYPE;
extern Flex<Type> USIZE_TYPE;
extern Flex<Type> FLOAT_TYPE;
extern Flex<Type> DOUBLE_TYPE;
extern Flex<Type> BOOL_TYPE;
extern Flex<Type> CHAR_TYPE;
extern Flex<Type> STR_TYPE;
extern Flex<Type> STR_REF_TYPE;
extern Flex<Type> NULL_TYPE;
extern Flex<Type> NEVER_TYPE;
extern Flex<Type> UNKNOWN_TYPE;

bool is_unknown_type(const Type &type);
bool is_never_type(const Type &type);
bool is_null_type(const Type &type);
bool is_float_type(const Type &type);
bool is_double_type(const Type &type);
bool is_usize_type(const Type &type);
bool is_bool_type(const Type &type);

} // namespace amelia
