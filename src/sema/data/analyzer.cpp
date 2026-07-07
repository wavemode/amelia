#include <climits>

#include "analyzer.hpp"

#include "lexer/data/lexer.hpp"
#include "sema/data/expression.hpp"
#include "source/data/source_location_error.hpp"
#include "util/data/integer.hpp"
#include "util/data/rational.hpp"
#include "util/data/slice_utils.hpp"
#include "util/data/text_utils.hpp"
#include "sema/data/sema_result.hpp"

namespace amelia {

namespace {

Flex<Type> BYTE_TYPE;
Flex<Type> UBYTE_TYPE;
Flex<Type> SHORT_TYPE;
Flex<Type> USHORT_TYPE;
Flex<Type> INT_TYPE;
Flex<Type> UINT_TYPE;
Flex<Type> LONG_TYPE;
Flex<Type> ULONG_TYPE;
Flex<Type> USIZE_TYPE;
Flex<Type> FLOAT_TYPE;
Flex<Type> DOUBLE_TYPE;
Flex<Type> BOOL_TYPE;
Flex<Type> CHAR_TYPE;
Flex<Type> STR_TYPE;
Flex<Type> STR_REF_TYPE;
Flex<Type> NULL_TYPE;
Flex<Type> NEVER_TYPE;
Flex<Type> UNKNOWN_TYPE;

bool init = []() {
  BYTE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Byte);
  UBYTE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UByte);
  SHORT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Short);
  USHORT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UShort);
  INT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Int);
  UINT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UInt);
  LONG_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Long);
  ULONG_TYPE = emplace_flex<BuiltinType>(BuiltinKind::ULong);
  USIZE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::USize);
  FLOAT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Float);
  DOUBLE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Double);
  BOOL_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Bool);
  CHAR_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Char);
  STR_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Str);
  NULL_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Null);
  NEVER_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Never);
  UNKNOWN_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Unknown);
  auto str_ref_type = emplace_flex<ReferenceType>();
  str_ref_type->referent = STR_TYPE;
  str_ref_type->is_const = false;
  str_ref_type->is_move = false;
  STR_REF_TYPE = move(str_ref_type);
  return true;
}();

class SemaWorkerState {
public:
  SemaWorkerState(SemaResult &sema_result, Module &module_obj)
      : m_sema_result(sema_result), m_module_obj(module_obj) {}

  void push_binding(Flex<Binding> binding) {
    Scope &scope = *m_module_obj.scope;
    Text name = binding->name;
    const Option<BindingId> existing_binding_id = scope.active_binding_ids.find(name);
    BindingId new_binding_id = scope.active_bindings.size();
    binding->shadowed_binding_id = existing_binding_id;
    binding->id = new_binding_id;
    scope.active_bindings.push_back(binding);
    scope.active_binding_ids.set(name, new_binding_id);
    if (m_binding_currently_analyzing.has_value()) {
      m_binding_currently_analyzing.value()->child_bindings.push_back(binding);
    }
  }

  Binding &pop_binding() {
    Scope &scope = *m_module_obj.scope;
    if (scope.active_bindings.size() == 0) {
      throw RuntimeError("Attempted to pop binding from empty scope");
    }
    Binding &binding = scope.active_bindings[scope.active_bindings.size() - 1];
    if (!is_binding_analyzed(binding)) {
      // TODO: warn about unused and un-analyzed?
    }
    if (!binding.shadowed_binding_id.has_value()) {
      scope.active_binding_ids.remove(binding.name);
    } else {
      scope.active_binding_ids.set(binding.name, binding.shadowed_binding_id.value());
    }
    scope.active_bindings.pop_back();
    return binding;
  }

  size_t get_binding_stack_size() const {
    return m_module_obj.scope->active_bindings.size();
  }

  void restore_binding_stack(size_t size) {
    Scope &scope = *m_module_obj.scope;
    while (scope.active_bindings.size() > size) {
      pop_binding();
    }
  }

  void initialize_binding(Text name, Flex<Expression> value) {
    Binding &binding = resolve_value_binding(value->node_id, name);
    if (binding.kind != BindingKind::Variable) {
      String error_message = "Attempted to initialize non-variable binding '";
      error_message.append(name);
      error_message.append("'");
      throw RuntimeError(error_message.c_str());
    }
    ValueBinding &value_binding = static_cast<ValueBinding &>(binding);
    if (value_binding.value.has_value()) {
      return;
    }
    Flex<ValueBinding> new_binding = emplace_flex<ValueBinding>(value_binding);
    if (is_unknown_type(new_binding->type.value())) {
      value_binding.type = value->type;
      new_binding->type = value->type;
    }
    new_binding->value = require_coerce(new_binding->type.value(), value);
    push_binding(move(new_binding));
  }

  bool is_trivial_type(Type &) {
    // TODO
    return true;
  }

  bool is_unknown_type(const Type &type) {
    return type.kind == TypeKind::Builtin &&
           static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Unknown;
  }

  bool is_never_type(const Type &type) {
    return type.kind == TypeKind::Builtin &&
           static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Never;
  }

  bool is_null_type(const Type &type) {
    return type.kind == TypeKind::Builtin &&
           static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Null;
  }

  bool is_float_type(const Type &type) {
    return type.kind == TypeKind::Builtin &&
           (static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Float);
  }

  bool is_double_type(const Type &type) {
    return type.kind == TypeKind::Builtin &&
           (static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Double);
  }

  bool is_integral_type(const Type &type) {
    if (type.kind == TypeKind::ConstInteger || type.kind == TypeKind::BitInt ||
        type.kind == TypeKind::ConstCharacter) {
      return true;
    }
    if (type.kind != TypeKind::Builtin) {
      return false;
    }
    switch (static_cast<const BuiltinType &>(type).builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Char:
      return true;
    case BuiltinKind::Bool:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return false;
    }
  }

  bool is_value_binding_node_type(NodeType node_type) {
    return node_type == NodeType::LetDeclNode || node_type == NodeType::ConstDeclNode ||
           node_type == NodeType::FunctionDeclNode;
  }

  bool is_type_binding_node_type(NodeType node_type) {
    // TODO
    return node_type == NodeType::TypeDeclNode;
  }

  bool type_has_builtin_numeric_representation(Type &type) {
    if (type.kind == TypeKind::ConstInteger || type.kind == TypeKind::BitInt ||
        type.kind == TypeKind::ConstRational || type.kind == TypeKind::ConstBoolean) {
      return true;
    }
    if (type.kind != TypeKind::Builtin) {
      return false;
    }
    switch (static_cast<const BuiltinType &>(type).builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Char:
    case BuiltinKind::Null:
      return true;
    case BuiltinKind::Str:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return false;
    }
  }

  bool is_native_integral_type(Type &type) {
    return is_integral_type(type) && repr_bit_size(type) <= 64;
  }

  bool is_non_promoting_binary_op(BinaryOperatorKind op_kind) {
    switch (op_kind) {
      case BinaryOperatorKind::Add:
      case BinaryOperatorKind::Subtract:
      case BinaryOperatorKind::Multiply:
      case BinaryOperatorKind::Divide:
      case BinaryOperatorKind::BitwiseAnd:
      case BinaryOperatorKind::BitwiseOr:
      case BinaryOperatorKind::BitwiseXor:
      case BinaryOperatorKind::Equals:
      case BinaryOperatorKind::Greater:
      case BinaryOperatorKind::GreaterEquals:
      case BinaryOperatorKind::Less:
      case BinaryOperatorKind::LessEquals:
      case BinaryOperatorKind::Modulo:
      case BinaryOperatorKind::NotEquals:
      case BinaryOperatorKind::Or:
      case BinaryOperatorKind::And:
        return false;
      case BinaryOperatorKind::LeftShift:
      case BinaryOperatorKind::RightShift:
      case BinaryOperatorKind::Assignment:
      case BinaryOperatorKind::BitAndAssignment:
      case BinaryOperatorKind::BitOrAssignment:
      case BinaryOperatorKind::BitXorAssignment:
      case BinaryOperatorKind::DivAssignment:
      case BinaryOperatorKind::LShiftAssignment:
      case BinaryOperatorKind::ModAssignment:
      case BinaryOperatorKind::MulAssignment:
      case BinaryOperatorKind::RShiftAssignment:
      case BinaryOperatorKind::SubAssignment:
      case BinaryOperatorKind::AddAssignment:
        return true;
    }
  }

  Integer max_value_of_type(Type &type) {
    if (type.kind == TypeKind::Builtin) {
      switch (static_cast<const BuiltinType &>(type).builtin_kind) {
      case BuiltinKind::Byte:
        return Integer(INT8_MAX);
      case BuiltinKind::UByte:
        return Integer(UINT8_MAX);
      case BuiltinKind::Short:
        return Integer(INT16_MAX);
      case BuiltinKind::UShort:
        return Integer(UINT16_MAX);
      case BuiltinKind::Int:
        return Integer(INT32_MAX);
      case BuiltinKind::UInt:
        return Integer(UINT32_MAX);
      case BuiltinKind::Long:
        return Integer(INT64_MAX);
      case BuiltinKind::ULong:
        return Integer(UINT64_MAX);
      case BuiltinKind::USize:
        return Integer(UINT64_MAX);
      case BuiltinKind::Char:
        return Integer(0x10FFFF);
      default:
        break;
      }
    } else if (type.kind == TypeKind::ConstInteger) {
      return static_cast<const ConstIntegerType &>(type).value;
    } else if (type.kind == TypeKind::ConstCharacter) {
      return Integer(static_cast<const ConstCharacterType &>(type).value);
    } else if (type.kind == TypeKind::BitInt) {
      const auto &bitint_type = static_cast<const BitIntType &>(type);
      if (bitint_type.is_signed) {
        return (Integer(1) << (bitint_type.bit_width - 1)) - 1;
      } else {
        return (Integer(1) << bitint_type.bit_width) - 1;
      }
    }
    throw RuntimeError("Cannot determine max value of type");
  }

  Integer min_value_of_type(Type &type) {
    if (type.kind == TypeKind::Builtin) {
      switch (static_cast<const BuiltinType &>(type).builtin_kind) {
      case BuiltinKind::Byte:
        return Integer(INT8_MIN);
      case BuiltinKind::UByte:
        return Integer(0);
      case BuiltinKind::Short:
        return Integer(INT16_MIN);
      case BuiltinKind::UShort:
        return Integer(0);
      case BuiltinKind::Int:
        return Integer(INT32_MIN);
      case BuiltinKind::UInt:
        return Integer(0);
      case BuiltinKind::Long:
        return Integer(INT64_MIN);
      case BuiltinKind::ULong:
        return Integer(0);
      case BuiltinKind::USize:
        return Integer(0);
      case BuiltinKind::Char:
        return Integer(0);
      default:
        break;
      }
    } else if (type.kind == TypeKind::ConstInteger) {
      return static_cast<const ConstIntegerType &>(type).value;
    } else if (type.kind == TypeKind::ConstCharacter) {
      return Integer(static_cast<const ConstCharacterType &>(type).value);
    } else if (type.kind == TypeKind::BitInt) {
      const auto &bitint_type = static_cast<const BitIntType &>(type);
      if (bitint_type.is_signed) {
        return -(Integer(1) << (bitint_type.bit_width - 1));
      } else {
        return Integer(0);
      }
    }
    throw RuntimeError("Cannot determine min value of type");
  }

  uint32_t repr_bit_size(Type &type) {
    if (type.kind == TypeKind::ConstInteger) {
      // ConstIntegers with values within the int32_t range are represented as int32_t
      // Any larger than that, and we use the smallest power of 2 which can represent the value
      // in signed two's complement.
      const auto &const_int_type = static_cast<const ConstIntegerType &>(type);
      if (const_int_type.value >= INT32_MIN && const_int_type.value <= INT32_MAX) {
        return 32;
      }

      uint32_t bit_width = 1;
      Integer abs_value = const_int_type.value.abs();
      while (abs_value > 0) {
        abs_value /= 2;
        ++bit_width;
      }

      uint32_t next_power_of_two = 1;
      while (next_power_of_two < bit_width) {
        next_power_of_two <<= 1;
      }
      return next_power_of_two;
    } else if (type.kind == TypeKind::Builtin) {
      switch (static_cast<const BuiltinType &>(type).builtin_kind) {
      case BuiltinKind::Byte:
      case BuiltinKind::UByte:
        return 8;
      case BuiltinKind::Short:
      case BuiltinKind::UShort:
        return 16;
      case BuiltinKind::Int:
      case BuiltinKind::UInt:
      case BuiltinKind::Char:
        return 32;
      case BuiltinKind::Long:
      case BuiltinKind::ULong:
        return 64;
      case BuiltinKind::USize:
        return 64;
      default:
        throw RuntimeError("Cannot determine representation bit size of type");
      }
    } else if (type.kind == TypeKind::BitInt) {
      const auto &bitint_type = static_cast<const BitIntType &>(type);
      return bitint_type.bit_width;
    } else {
      throw RuntimeError("Cannot determine representation bit size of type");
    }
  }

  bool can_type_represent_range(Type &type, const Integer &min, const Integer &max) {
    return min_value_of_type(type) <= min && max_value_of_type(type) >= max;
  }

  Flex<Expression> builtin_type_coerce(Flex<Type> target_type, Flex<Expression> expr) {
    auto coerce_expr = emplace_flex<BuiltinTypeCoerceExpression>();
    coerce_expr->type = target_type;
    coerce_expr->expr = move(expr);
    return coerce_expr;
  }

  Flex<Expression> builtin_type_cast(Flex<Type> target_type, Flex<Expression> expr) {
    auto cast_expr = emplace_flex<BuiltinTypeCastExpression>();
    cast_expr->type = target_type;
    cast_expr->expr = move(expr);
    return cast_expr;
  }

  Flex<Type> char_to_uint(Flex<Type> type) {
    if (type->kind == TypeKind::ConstCharacter ||
        (type->kind == TypeKind::Builtin &&
         static_cast<const BuiltinType &>(*type).builtin_kind == BuiltinKind::Char)) {
      return UINT_TYPE;
    }
    return type;
  }

  Flex<Type> remove_const(Flex<Type> type) {
    switch (type->kind) {
    case TypeKind::Alias:
      return remove_const(type.derive(static_cast<AliasType &>(*type)));
    case TypeKind::Reference:
      return remove_const(type.derive(static_cast<ReferenceType &>(*type)));
    case TypeKind::Struct:
      throw RuntimeError("not implemented (remove_const(Struct))");
    case TypeKind::Tuple:
      return remove_const(type.derive(static_cast<TupleType &>(*type)));
    case TypeKind::Array:
      return remove_const(type.derive(static_cast<ArrayType &>(*type)));
    case TypeKind::TypeFn:
      throw RuntimeError("not implemented (remove_const(TypeFn))");
    case TypeKind::Apply:
      throw RuntimeError("not implemented (remove_const(Apply))");
    case TypeKind::Builtin:
      return remove_const(type.derive(static_cast<BuiltinType &>(*type)));
    case TypeKind::BitInt:
      return remove_const(type.derive(static_cast<BitIntType &>(*type)));
    case TypeKind::Pointer:
      return remove_const(type.derive(static_cast<PointerType &>(*type)));
    case TypeKind::Slice:
      return remove_const(type.derive(static_cast<SliceType &>(*type)));
    case TypeKind::Impl:
      throw RuntimeError("not implemented (remove_const(Impl))");
    case TypeKind::ConstInteger:
      return remove_const(type.derive(static_cast<ConstIntegerType &>(*type)));
    case TypeKind::ConstRational:
      return remove_const(type.derive(static_cast<ConstRationalType &>(*type)));
    case TypeKind::ConstBoolean:
      return remove_const(type.derive(static_cast<ConstBooleanType &>(*type)));
    case TypeKind::ConstCharacter:
      return remove_const(type.derive(static_cast<ConstCharacterType &>(*type)));
    case TypeKind::ConstString:
      return remove_const(type.derive(static_cast<ConstStringType &>(*type)));
    case TypeKind::Class:
      throw RuntimeError("not implemented (remove_const(Class))");
    case TypeKind::Union:
      throw RuntimeError("not implemented (remove_const(Union))");
    case TypeKind::Concept:
      throw RuntimeError("not implemented (remove_const(Concept))");
    case TypeKind::Function:
      throw RuntimeError("not implemented (remove_const(Function))");
    case TypeKind::FunctionPointer:
      throw RuntimeError("not implemented (remove_const(FunctionPointer))");
    case TypeKind::Closure:
      throw RuntimeError("not implemented (remove_const(Closure))");
    case TypeKind::Variable:
      throw RuntimeError("not implemented (remove_const(Variable))");
    }
  }

  Flex<Type> remove_const(Flex<AliasType> alias_type) {
    return remove_const(alias_type->target);
  }

  Flex<Type> remove_const(Flex<ReferenceType> reference_type) {
    auto result = emplace_flex<ReferenceType>();
    result->referent = remove_const(reference_type->referent);
    result->is_const = reference_type->is_const;
    result->is_move = reference_type->is_move;
    return result;
  }

  Flex<Type> remove_const(Flex<TupleType> tuple_type) {
    auto result = emplace_flex<TupleType>();
    for (const auto &element_type : tuple_type->element_types) {
      result->element_types.push_back(remove_const(element_type));
    }
    return result;
  }

  Flex<Type> remove_const(Flex<ArrayType> array_type) {
    auto result = emplace_flex<ArrayType>();
    result->element_type = remove_const(array_type->element_type);
    result->size = array_type->size;
    return result;
  }

  Flex<Type> remove_const(Flex<BuiltinType> builtin_type) {
    return builtin_type;
  }

  Flex<Type> remove_const(Flex<BitIntType> bitint_type) {
    return bitint_type;
  }

  Flex<Type> remove_const(Flex<PointerType> pointer_type) {
    auto result = emplace_flex<PointerType>();
    result->pointee = remove_const(pointer_type->pointee);
    result->is_const = pointer_type->is_const;
    return result;
  }

  Flex<Type> remove_const(Flex<SliceType> slice_type) {
    auto result = emplace_flex<SliceType>();
    result->element_type = remove_const(slice_type->element_type);
    return result;
  }

  Flex<Type> remove_const(Flex<ConstIntegerType> const_integer_type) {
    auto bit_width = repr_bit_size(*const_integer_type);
    if (bit_width == 32) {
      return INT_TYPE;
    } else if (bit_width == 64) {
      return LONG_TYPE;
    } else {
      auto result = emplace_flex<BitIntType>();
      result->bit_width = repr_bit_size(*const_integer_type);
      result->is_signed = true;
      return result;
    }
  }

  Flex<Type> remove_const(Flex<ConstRationalType>) {
    return DOUBLE_TYPE;
  }

  Flex<Type> remove_const(Flex<ConstBooleanType>) {
    return BOOL_TYPE;
  }

  Flex<Type> remove_const(Flex<ConstCharacterType>) {
    return CHAR_TYPE;
  }

  Flex<Type> remove_const(Flex<ConstStringType>) {
    return STR_REF_TYPE;
  }

  bool unify(Flex<Type> target_type, Flex<Type> assignment_type) {
    resolve_update(assignment_type);

    if (&*target_type == &*assignment_type) {
      return true;
    }

    if (assignment_type->kind == TypeKind::Builtin &&
        static_cast<BuiltinType &>(*assignment_type).builtin_kind == BuiltinKind::Never) {
      return true;
    }

    switch (target_type->kind) {
    case TypeKind::Alias:
      return unify(target_type.derive(static_cast<AliasType &>(*target_type)), assignment_type);
    case TypeKind::Reference:
      return unify(target_type.derive(static_cast<ReferenceType &>(*target_type)), assignment_type);
    case TypeKind::Struct:
      throw RuntimeError("not implemented (unify(Struct))");
    case TypeKind::Tuple:
      return unify(target_type.derive(static_cast<TupleType &>(*target_type)), assignment_type);
    case TypeKind::Array:
      return unify(target_type.derive(static_cast<ArrayType &>(*target_type)), assignment_type);
    case TypeKind::TypeFn:
      throw RuntimeError("not implemented (unify(TypeFn))");
    case TypeKind::Apply:
      throw RuntimeError("not implemented (unify(Apply))");
    case TypeKind::Builtin:
      return unify(target_type.derive(static_cast<BuiltinType &>(*target_type)), assignment_type);
    case TypeKind::BitInt:
      return unify(target_type.derive(static_cast<BitIntType &>(*target_type)), assignment_type);
    case TypeKind::Pointer:
      return unify(target_type.derive(static_cast<PointerType &>(*target_type)), assignment_type);
    case TypeKind::Slice:
      return unify(target_type.derive(static_cast<SliceType &>(*target_type)), assignment_type);
    case TypeKind::Impl:
      throw RuntimeError("not implemented (unify(Impl))");
    case TypeKind::ConstInteger:
      return unify(
          target_type.derive(static_cast<ConstIntegerType &>(*target_type)), assignment_type
      );
    case TypeKind::ConstRational:
      return unify(
          target_type.derive(static_cast<ConstRationalType &>(*target_type)), assignment_type
      );
    case TypeKind::ConstBoolean:
      return unify(
          target_type.derive(static_cast<ConstBooleanType &>(*target_type)), assignment_type
      );
    case TypeKind::ConstCharacter:
      return unify(
          target_type.derive(static_cast<ConstCharacterType &>(*target_type)), assignment_type
      );
    case TypeKind::ConstString:
      return unify(
          target_type.derive(static_cast<ConstStringType &>(*target_type)), assignment_type
      );
    case TypeKind::Class:
      throw RuntimeError("not implemented (unify(Class))");
    case TypeKind::Union:
      throw RuntimeError("not implemented (unify(Union))");
    case TypeKind::Concept:
      throw RuntimeError("not implemented (unify(Concept))");
    case TypeKind::Function:
      throw RuntimeError("not implemented (unify(Function))");
    case TypeKind::FunctionPointer:
      throw RuntimeError("not implemented (unify(FunctionPointer))");
    case TypeKind::Closure:
      throw RuntimeError("not implemented (unify(Closure))");
    case TypeKind::Variable:
      throw RuntimeError("not implemented (unify(Variable))");
    }
  }

  bool unify(Flex<AliasType> target_type, Flex<Type> assignment_type) {
    return unify(target_type->target, assignment_type);
  }

  bool unify(Flex<ReferenceType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind == TypeKind::ConstString) {
      return unify(target_type, STR_REF_TYPE);
    }

    if (assignment_type->kind != TypeKind::Reference) {
      return false;
    }
    auto &assignment_type_ref = static_cast<ReferenceType &>(*assignment_type);
    if (!unify(target_type->referent, assignment_type_ref.referent)) {
      return false;
    }
    return target_type->is_const == assignment_type_ref.is_const &&
           target_type->is_move == assignment_type_ref.is_move;
  }

  bool unify(Flex<TupleType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind != TypeKind::Tuple) {
      return false;
    }
    TupleType &assignment_tuple = static_cast<TupleType &>(*assignment_type);
    if (target_type->element_types.size() != assignment_tuple.element_types.size()) {
      return false;
    }
    for (size_t i = 0; i < target_type->element_types.size(); ++i) {
      if (!unify(target_type->element_types[i], assignment_tuple.element_types[i])) {
        return false;
      }
    }
    return true;
  }

  bool unify(Flex<ArrayType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind != TypeKind::Array) {
      return false;
    }
    auto &assignment_array = static_cast<ArrayType &>(*assignment_type);
    if (!unify(target_type->element_type, assignment_array.element_type)) {
      return false;
    }
    return target_type->size == assignment_array.size;
  }

  bool unify(Flex<BuiltinType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind == TypeKind::Builtin) {
      return target_type->builtin_kind == static_cast<BuiltinType &>(*assignment_type).builtin_kind;
    } else if (assignment_type->kind == TypeKind::ConstInteger ||
               assignment_type->kind == TypeKind::BitInt) {
      return is_integral_type(target_type) && min_value_of_type(target_type) < 0 &&
             repr_bit_size(target_type) == repr_bit_size(assignment_type);
    } else if (assignment_type->kind == TypeKind::ConstRational) {
      return target_type->builtin_kind == BuiltinKind::Double;
    } else if (assignment_type->kind == TypeKind::ConstBoolean) {
      return target_type->builtin_kind == BuiltinKind::Bool;
    } else if (assignment_type->kind == TypeKind::ConstCharacter) {
      return target_type->builtin_kind == BuiltinKind::Char;
    }
    return false;
  }

  bool unify(Flex<BitIntType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind == TypeKind::ConstInteger) {
      return target_type->bit_width == repr_bit_size(assignment_type) && target_type->is_signed;
    }

    return is_integral_type(assignment_type) &&
           (max_value_of_type(target_type) == max_value_of_type(assignment_type) &&
            min_value_of_type(target_type) == min_value_of_type(assignment_type));
  }

  bool unify(Flex<PointerType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind != TypeKind::Pointer) {
      return false;
    }
    auto &assignment_type_ptr = static_cast<PointerType &>(*assignment_type);
    if (!unify(target_type->pointee, assignment_type_ptr.pointee)) {
      return false;
    }
    return target_type->is_const == assignment_type_ptr.is_const;
  }

  bool unify(Flex<SliceType> target_type, Flex<Type> assignment_type) {
    if (assignment_type->kind == TypeKind::Slice) {
      return unify(
          target_type->element_type, static_cast<SliceType &>(*assignment_type).element_type
      );
    }
    return false;
  }

  bool unify(Flex<ConstIntegerType> target_type, Flex<Type> assignment_type) {
    return assignment_type->kind == TypeKind::ConstInteger &&
           target_type->value == static_cast<const ConstIntegerType &>(*assignment_type).value;
  }

  bool unify(Flex<ConstRationalType> target_type, Flex<Type> assignment_type) {
    return assignment_type->kind == TypeKind::ConstRational &&
           target_type->value == static_cast<const ConstRationalType &>(*assignment_type).value;
  }

  bool unify(Flex<ConstBooleanType> target_type, Flex<Type> assignment_type) {
    return assignment_type->kind == TypeKind::ConstBoolean &&
           target_type->value == static_cast<const ConstBooleanType &>(*assignment_type).value;
  }

  bool unify(Flex<ConstCharacterType> target_type, Flex<Type> assignment_type) {
    return assignment_type->kind == TypeKind::ConstCharacter &&
           target_type->value == static_cast<const ConstCharacterType &>(*assignment_type).value;
  }

  bool unify(Flex<ConstStringType> target_type, Flex<Type> assignment_type) {
    return assignment_type->kind == TypeKind::ConstString &&
           target_type->value == static_cast<const ConstStringType &>(*assignment_type).value;
  }

  Type &resolve(Type &type) {
    switch (type.kind) {
    case TypeKind::Alias:
      return resolve(static_cast<AliasType &>(type));
    case TypeKind::TypeFn:
      throw RuntimeError("not implemented (resolve(TypeFn))");
    case TypeKind::Apply:
      throw RuntimeError("not implemented (resolve(Apply))");
    case TypeKind::Variable:
      throw RuntimeError("not implemented (resolve(Variable))");
    case TypeKind::BitInt:
    case TypeKind::Reference:
    case TypeKind::Struct:
    case TypeKind::Tuple:
    case TypeKind::Array:
    case TypeKind::Builtin:
    case TypeKind::Pointer:
    case TypeKind::Slice:
    case TypeKind::Impl:
    case TypeKind::ConstInteger:
    case TypeKind::ConstRational:
    case TypeKind::ConstBoolean:
    case TypeKind::ConstCharacter:
    case TypeKind::ConstString:
    case TypeKind::Class:
    case TypeKind::Union:
    case TypeKind::Concept:
    case TypeKind::Function:
    case TypeKind::FunctionPointer:
    case TypeKind::Closure:
      return type;
    }
  }

  Type &resolve(AliasType &alias_type) {
    return resolve(*alias_type.target);
  }

  void resolve_update(Flex<Type> &type) {
    auto &resolved_type = resolve(type);
    if (&resolved_type != &*type) {
      type = type.derive(resolved_type);
    }
  }

  Option<Flex<Expression>> cast(
      Flex<Type> target_type, Flex<Type> source_type, Flex<Expression> expr
  ) {
    resolve_update(source_type);

    if (unify(target_type, source_type)) {
      return expr;
    }

    auto coerced = coerce(target_type, source_type, expr);
    if (coerced.has_value()) {
      return coerced.value();
    }

    // TODO: source type `operator as`

    switch (target_type->kind) {
    case TypeKind::Alias:
      return cast(target_type.derive(static_cast<AliasType &>(*target_type)), source_type, expr);
    case TypeKind::Reference:
      throw RuntimeError("not implemented (cast(Reference))");
    case TypeKind::Struct:
      throw RuntimeError("not implemented (coerce(Struct))");
    case TypeKind::Tuple:
      throw RuntimeError("not implemented (coerce(Tuple))");
    case TypeKind::Array:
      throw RuntimeError("not implemented (coerce(Array))");
    case TypeKind::TypeFn:
      throw RuntimeError("not implemented (coerce(TypeFn))");
    case TypeKind::Apply:
      throw RuntimeError("not implemented (coerce(Apply))");
    case TypeKind::Builtin:
      return cast(target_type.derive(static_cast<BuiltinType &>(*target_type)), source_type, expr);
    case TypeKind::BitInt:
      return cast(target_type.derive(static_cast<BitIntType &>(*target_type)), source_type, expr);
    case TypeKind::Pointer:
      throw RuntimeError("not implemented (coerce(Pointer))");
    case TypeKind::Slice:
      throw RuntimeError("not implemented (coerce(Slice))");
    case TypeKind::Impl:
      throw RuntimeError("not implemented (coerce(Impl))");
    case TypeKind::ConstInteger:
      throw RuntimeError("not implemented (coerce(ConstInteger))");
    case TypeKind::ConstRational:
      throw RuntimeError("not implemented (coerce(ConstRational))");
    case TypeKind::ConstBoolean:
      throw RuntimeError("not implemented (coerce(ConstBoolean))");
    case TypeKind::ConstCharacter:
      throw RuntimeError("not implemented (coerce(ConstCharacter))");
    case TypeKind::ConstString:
      throw RuntimeError("not implemented (coerce(ConstString))");
    case TypeKind::Class:
      throw RuntimeError("not implemented (coerce(Class))");
    case TypeKind::Union:
      throw RuntimeError("not implemented (coerce(Union))");
    case TypeKind::Concept:
      throw RuntimeError("not implemented (coerce(Concept))");
    case TypeKind::Function:
      throw RuntimeError("not implemented (coerce(Function))");
    case TypeKind::FunctionPointer:
      throw RuntimeError("not implemented (coerce(FunctionPointer))");
    case TypeKind::Closure:
      throw RuntimeError("not implemented (coerce(Closure))");
    case TypeKind::Variable:
      throw RuntimeError("not implemented (coerce(Variable))");
    }
  }

  Option<Flex<Expression>> cast(
      Flex<AliasType> target_type, Flex<Type> source_type, Flex<Expression> expr
  ) {
    return cast(target_type->target, source_type, expr);
  }

  Option<Flex<Expression>> cast(
      Flex<BuiltinType> target_type, Flex<Type> source_type, Flex<Expression> expr
  ) {
    if (target_type->builtin_kind != BuiltinKind::Char &&
        type_has_builtin_numeric_representation(target_type) &&
        type_has_builtin_numeric_representation(source_type)) {
      return builtin_type_cast(target_type, move(expr));
    }
    return None();
  }

  Option<Flex<Expression>> cast(
      Flex<BitIntType> target_type, Flex<Type> source_type, Flex<Expression> expr
  ) {
    if (type_has_builtin_numeric_representation(source_type)) {
      return builtin_type_cast(target_type, move(expr));
    }
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<Type> target_type, Flex<Expression> expr) {
    return coerce(target_type, expr->type, expr);
  }

  Option<Flex<Expression>> coerce(
      Flex<Type> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    if (unify(target_type, assignment_type)) {
      return expr;
    }

    resolve_update(assignment_type);

    // TODO: source type implicit `operator as`

    switch (target_type->kind) {
    case TypeKind::Alias:
      return coerce(
          target_type.derive(static_cast<AliasType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Reference:
      return coerce(
          target_type.derive(static_cast<ReferenceType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Struct:
      throw RuntimeError("not implemented (coerce(Struct))");
    case TypeKind::Tuple:
      return coerce(
          target_type.derive(static_cast<TupleType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Array:
      throw RuntimeError("not implemented (coerce(Array))");
    case TypeKind::TypeFn:
      throw RuntimeError("not implemented (coerce(TypeFn))");
    case TypeKind::Apply:
      throw RuntimeError("not implemented (coerce(Apply))");
    case TypeKind::Builtin:
      return coerce(
          target_type.derive(static_cast<BuiltinType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::BitInt:
      return coerce(
          target_type.derive(static_cast<BitIntType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Pointer:
      return coerce(
          target_type.derive(static_cast<PointerType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Slice:
      return coerce(
          target_type.derive(static_cast<SliceType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Impl:
      throw RuntimeError("not implemented (coerce(Impl))");
    case TypeKind::ConstInteger:
      return coerce(
          target_type.derive(static_cast<ConstIntegerType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::ConstRational:
      return coerce(
          target_type.derive(static_cast<ConstRationalType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::ConstBoolean:
      return coerce(
          target_type.derive(static_cast<ConstBooleanType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::ConstCharacter:
      return coerce(
          target_type.derive(static_cast<ConstCharacterType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::ConstString:
      return coerce(
          target_type.derive(static_cast<ConstStringType &>(*target_type)), assignment_type, expr
      );
    case TypeKind::Class:
      throw RuntimeError("not implemented (coerce(Class))");
    case TypeKind::Union:
      throw RuntimeError("not implemented (coerce(Union))");
    case TypeKind::Concept:
      throw RuntimeError("not implemented (coerce(Concept))");
    case TypeKind::Function:
      throw RuntimeError("not implemented (coerce(Function))");
    case TypeKind::FunctionPointer:
      throw RuntimeError("not implemented (coerce(FunctionPointer))");
    case TypeKind::Closure:
      throw RuntimeError("not implemented (coerce(Closure))");
    case TypeKind::Variable:
      throw RuntimeError("not implemented (coerce(Variable))");
    }
  }

  Option<Flex<Expression>> coerce(
      Flex<AliasType> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    return coerce(target_type->target, assignment_type, expr);
  }

  Option<Flex<Expression>> coerce(
      Flex<ReferenceType> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    if (assignment_type->kind == TypeKind::Reference) {
      auto &expr_ref_type = static_cast<ReferenceType &>(*assignment_type);
      if (
          // If target is not const, assignment must also not be const
          (target_type->is_const || !expr_ref_type.is_const) &&

          // If target type is non-trivial, "move" quality of references must be the same
          (is_trivial_type(target_type->referent) ||
           (target_type->is_move == expr_ref_type.is_move))
      ) {
        if (
            // References refer to the same type
            // TODO: compatible types
            unify(target_type->referent, expr_ref_type.referent)
        ) {
          return builtin_type_coerce(target_type, move(expr));
        } else if (
            // Target type refers to a slice and expr type refers to an array of the same type
            target_type->referent->kind == TypeKind::Slice &&
            expr_ref_type.referent->kind == TypeKind::Array &&
            unify(
                static_cast<SliceType &>(*target_type->referent).element_type,
                static_cast<ArrayType &>(*expr_ref_type.referent).element_type
            )
        ) {
          return builtin_type_coerce(target_type, move(expr));
        }
      }
    } else if (assignment_type->kind == TypeKind::ConstString) {
      if (target_type->referent->kind == TypeKind::Builtin &&
          static_cast<const BuiltinType &>(*target_type->referent).builtin_kind ==
              BuiltinKind::Str) {
        return builtin_type_coerce(target_type, move(expr));
      }
    }
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<TupleType>, Flex<Type>, Flex<Expression>) {
    // TODO: compatible tuples
    return None();
  }

  Option<Flex<Expression>> coerce(
      Flex<BuiltinType> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    switch (target_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
      return is_integral_type(assignment_type) && can_type_represent_range(
                                                      *target_type,
                                                      min_value_of_type(assignment_type),
                                                      max_value_of_type(assignment_type)
                                                  )
                 ? builtin_type_coerce(target_type, move(expr))
                 : Option<Flex<Expression>>();
    case BuiltinKind::Float:
      return is_integral_type(assignment_type) ||
                     (assignment_type->kind == TypeKind::ConstRational) ||
                     (assignment_type->kind == TypeKind::Builtin &&
                      static_cast<const BuiltinType &>(*assignment_type).builtin_kind ==
                          BuiltinKind::Float)
                 ? builtin_type_coerce(target_type, move(expr))
                 : Option<Flex<Expression>>();
    case BuiltinKind::Double:
      return is_integral_type(assignment_type) ||
                     (assignment_type->kind == TypeKind::ConstRational) ||
                     (assignment_type->kind == TypeKind::Builtin &&
                      (static_cast<const BuiltinType &>(*assignment_type).builtin_kind ==
                           BuiltinKind::Float ||
                       static_cast<const BuiltinType &>(*assignment_type).builtin_kind ==
                           BuiltinKind::Double))
                 ? builtin_type_coerce(target_type, move(expr))
                 : Option<Flex<Expression>>();
    case BuiltinKind::Bool:
      return (assignment_type->kind == TypeKind::ConstBoolean) ||
                     (assignment_type->kind == TypeKind::Builtin &&
                      static_cast<const BuiltinType &>(*assignment_type).builtin_kind ==
                          BuiltinKind::Bool)
                 ? builtin_type_coerce(target_type, move(expr))
                 : Option<Flex<Expression>>();
    case BuiltinKind::Char:
      return ((assignment_type->kind == TypeKind::ConstInteger &&
               (static_cast<const ConstIntegerType &>(*assignment_type).value <= UINT32_MAX &&
                static_cast<const ConstIntegerType &>(*assignment_type).value >= 0 &&
                CharIterator::is_valid_code_point(
                    static_cast<const ConstIntegerType &>(*assignment_type).value.to_uint32()
                ))) ||
              (is_integral_type(assignment_type) && min_value_of_type(assignment_type) >= 0 &&
               max_value_of_type(assignment_type) < 0xD800))
                 ? builtin_type_coerce(target_type, move(expr))
                 : Option<Flex<Expression>>();
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> coerce(
      Flex<BitIntType> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    if (is_integral_type(assignment_type) &&
        can_type_represent_range(
            *target_type, min_value_of_type(assignment_type), max_value_of_type(assignment_type)
        )) {
      return builtin_type_coerce(target_type, move(expr));
    }

    return None();
  }

  Option<Flex<Expression>> coerce(
      Flex<PointerType> target_type, Flex<Type> assignment_type, Flex<Expression> expr
  ) {
    if (assignment_type->kind == TypeKind::Pointer) {
      auto &expr_ptr_type = static_cast<PointerType &>(*assignment_type);
      if (
          // Pointers point to the same type
          unify(target_type->pointee, expr_ptr_type.pointee) &&
          // If assignment is const, target must also be const
          (target_type->is_const || !expr_ptr_type.is_const)
      ) {
        return builtin_type_coerce(target_type, move(expr));
      }
    }
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<SliceType>, Flex<Type>, Flex<Expression>) {
    // slices themselves can't be coerced
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<ConstIntegerType>, Flex<Type>, Flex<Expression>) {
    // by this point, we already know that assignment_type is not an equivalent ConstInteger
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<ConstRationalType>, Flex<Type>, Flex<Expression>) {
    // by this point, we already know that assignment_type is not an equivalent ConstRational
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<ConstBooleanType>, Flex<Type>, Flex<Expression>) {
    // by this point, we already know that assignment_type is not an equivalent ConstBoolean
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<ConstCharacterType>, Flex<Type>, Flex<Expression>) {
    // by this point, we already know that assignment_type is not an equivalent ConstCharacter
    return None();
  }

  Option<Flex<Expression>> coerce(Flex<ConstStringType>, Flex<Type>, Flex<Expression>) {
    // by this point, we already know that assignment_type is not an equivalent ConstString
    return None();
  }

  Flex<Expression> require_coerce(Flex<Type> target_type, Flex<Expression> expr) {
    return require_coerce(
        target_type, expr, "Cannot coerce expression of type '{1}' to expected type '{2}'"
    );
  }

  Flex<Expression> require_coerce(
      Flex<Type> target_type, Flex<Expression> expr, String &&error_message_template
  ) {
    auto unified_expr = coerce(target_type, expr->type, expr);
    if (!unified_expr.has_value()) {
      String target_type_str;
      target_type->serialize().to_string(target_type_str);
      String expr_type_str;
      expr->type->serialize().to_string(expr_type_str);
      TextUtils::replace(error_message_template, "{1}", expr_type_str);
      TextUtils::replace(error_message_template, "{2}", target_type_str);
      raise_error_at_node_id(expr->node_id, move(error_message_template));
    }
    return unified_expr.value();
  }

  void analyze_module() {
    m_module_obj.analyzed = true;
    collect_top_level_bindings();
    List<Text> binding_names;
    for (const auto &[binding_name, binding_id] : m_module_obj.scope->active_binding_ids) {
      binding_names.push_back(binding_name);
    }
    binding_names.sort();
    for (Text binding_name : binding_names) {
      Binding
          &binding = *m_module_obj.scope
                          ->active_bindings[m_module_obj.scope->active_binding_ids[binding_name]];
      analyze_top_level_binding(binding);
    }
  }

  void analyze_top_level_binding(Binding &binding) {
    analyze_binding(binding);
    switch (binding.kind) {
    case BindingKind::Variable:
      disallow_shadowing(static_cast<ValueBinding &>(binding));
      require_initializer(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Constant:
      disallow_shadowing(static_cast<ValueBinding &>(binding));
      require_initializer(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Function:
      disallow_function_from_shadowing_non_function(static_cast<ValueBinding &>(binding));
      break;
    default:
      break;
    }
  }

  bool is_binding_analyzed(const Binding &binding) {
    switch (binding.kind) {
    case BindingKind::Variable:
    case BindingKind::Constant:
    case BindingKind::Function:
      return static_cast<const ValueBinding &>(binding).type.has_value() &&
             !is_unknown_type(static_cast<const ValueBinding &>(binding).type.value());
      break;
    case BindingKind::Type:
      return static_cast<const TypeBinding &>(binding).type.has_value() &&
             !is_unknown_type(static_cast<const TypeBinding &>(binding).type.value());
      break;
    default:
      raise_error_at_node_id(
          binding.decl, "not implemented (unknown binding kind in is_binding_analyzed)"
      );
    }
  }

  void analyze_binding(Binding &binding) {
    auto old_binding_currently_analyzing = m_binding_currently_analyzing;
    m_binding_currently_analyzing = &binding;

    switch (binding.kind) {
    case BindingKind::Variable:
      analyze_let_binding(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Constant:
      analyze_const_binding(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Function:
      analyze_function_binding(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Type:
      analyte_type_binding(static_cast<TypeBinding &>(binding));
      break;
    default:
      raise_error_at_node_id(
          binding.decl, "not implemented (unknown binding kind in analyze_binding)"
      );
    }

    m_binding_currently_analyzing = old_binding_currently_analyzing;
  }

  Flex<TypeBinding> resolve_type_binding(NodeId node_id, Text name) {
    const Option<BindingId> binding_id = m_module_obj.scope->active_binding_ids.find(name);
    if (!binding_id.has_value()) {
      String error_message = "Unknown type name '";
      error_message.append(name);
      error_message.append("'");
      raise_error_at_node_id(node_id, move(error_message));
    }
    Flex<Binding> binding = m_module_obj.scope->active_bindings[binding_id.value()];
    analyze_binding(binding);
    switch (binding->kind) {
    case BindingKind::Type:
      return binding.derive(static_cast<TypeBinding &>(*binding));
    default: {
      String error_message = "Identifier '";
      error_message.append(name);
      error_message.append("' is not a type name");
      raise_error_at_node_id(node_id, move(error_message));
    }
    }
  }

  Flex<ValueBinding> resolve_value_binding(NodeId node_id, Text name) {
    const Option<BindingId> binding_id = m_module_obj.scope->active_binding_ids.find(name);
    if (!binding_id.has_value()) {
      String error_message = "Unknown identifier '";
      error_message.append(name);
      error_message.append("'");
      raise_error_at_node_id(node_id, move(error_message));
    }
    Flex<Binding> binding = m_module_obj.scope->active_bindings[binding_id.value()];
    analyze_binding(binding);
    switch (binding->kind) {
    case BindingKind::Constant:
    case BindingKind::Variable:
    case BindingKind::Function:
      return binding.derive(static_cast<ValueBinding &>(*binding));
    default:
      raise_error_at_node_id(
          node_id, "not implemented (unknown binding kind in resolve_value_binding)"
      );
    }
  }

  void require_initializer(const ValueBinding &binding) {
    if (!binding.value.has_value()) {
      String error_message = "Missing initializer for '";
      error_message.append(binding.name);
      error_message.append("' at top level");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void disallow_shadowing(const ValueBinding &binding) {
    if (binding.shadowed_binding_id.has_value()) {
      String error_message = "Duplicate declaration of '";
      error_message.append(binding.name);
      error_message.append("'");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void disallow_function_from_shadowing_non_function(const ValueBinding &binding) {
    auto *current_binding = &binding;
    while (current_binding->shadowed_binding_id.has_value()) {
      auto &shadowed_binding = *m_module_obj.scope
                                    ->active_bindings[current_binding->shadowed_binding_id.value()];
      if (shadowed_binding.kind != BindingKind::Function) {
        String error_message = "Function declaration '";
        error_message.append(binding.name);
        error_message.append("' conflicts with previous declaration of the same name");
        raise_error_at_node_id(binding.decl, move(error_message));
      }
      current_binding = static_cast<ValueBinding *>(&shadowed_binding);
    }
  }

  void analyte_type_binding(TypeBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    binding.type = UNKNOWN_TYPE;

    const auto &node = m_module_obj.ast.get_node(binding.decl);
    if (node.type() != NodeType::TypeDeclNode) {
      raise_error_at_node_id(
          binding.decl, "not implemented (analyte_type_binding for non-TypeDecl node)"
      );
    }
    const auto &type_decl_node = node.as_TypeDeclNode();

    if (type_decl_node.type_expr.has_value()) {
      auto type = evaluate_type_expr(type_decl_node.type_expr.value());
      auto result = emplace_flex<AliasType>();
      result->name = binding.name;
      result->module_name = m_module_obj.name;
      result->target = type.derive(resolve(type));
      binding.type = result;
    } else {
      raise_error_at_node_id(
          binding.decl, "not implemented (analyte_type_binding for TypeDecl node without type_expr)"
      );
    }
  }

  void analyze_let_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    const auto &decl_node = m_module_obj.ast.get_node(binding.decl).as_LetDeclNode();

    if (decl_node.type.has_value()) {
      binding.type = evaluate_type_expr(decl_node.type.value());
      if (decl_node.expr.has_value()) {
        binding.value = expect_expression_of_type(binding.type.value(), decl_node.expr.value());
      }
    } else {
      binding.type = UNKNOWN_TYPE;
      if (decl_node.expr.has_value()) {
        binding.value = build_expression(decl_node.expr.value());
        binding.type = remove_const(binding.value.value()->type);
      }
    }
  }

  void analyze_const_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    const auto &decl_node = m_module_obj.ast.get_node(binding.decl).as_ConstDeclNode();

    if (decl_node.type.has_value()) {
      binding.type = evaluate_type_expr(decl_node.type.value());
      if (decl_node.expr.has_value()) {
        binding.value = expect_expression_of_type(binding.type.value(), decl_node.expr.value());
      } else {
        raise_error_at_node_id(binding.decl, "Missing initializer for constant declaration");
      }
    } else {
      binding.type = UNKNOWN_TYPE;
      if (decl_node.expr.has_value()) {
        binding.value = build_expression(decl_node.expr.value());
        binding.type = binding.value.value()->type;
      } else {
        raise_error_at_node_id(binding.decl, "Missing initializer for constant declaration");
      }
    }
  }

  void analyze_function_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    binding.type = UNKNOWN_TYPE;

    auto function_type = Flex<FunctionType>::emplace();
    function_type->name = binding.name;

    auto *current_binding = &binding;
    while (true) {
      const auto &decl_node = m_module_obj.ast.get_node(current_binding->decl)
                                  .as_FunctionDeclNode();
      function_type->signatures.push_back(analyze_function_signature(decl_node.signature));

      if (!current_binding->shadowed_binding_id.has_value()) {
        break;
      }

      auto &shadowed_binding = *m_module_obj.scope
                                    ->active_bindings[current_binding->shadowed_binding_id.value()];
      if (shadowed_binding.kind != BindingKind::Function ||
          static_cast<const ValueBinding &>(shadowed_binding).type.has_value()) {
        // The function declaration we're currently analyzing is shadowing a previous declaration
        // that either isn't a function, or was already analyzed. This generally happens when a
        // local declaration shadows a local or global one.

        // Either way, it should not be part of this overload set.
        break;
      }

      if (shadowed_binding.id.value() != current_binding->id.value() - 1) {
        String error_message = "Overload of function '";
        error_message.append(binding.name);
        error_message.append("' must be declared adjacent to its other overloads");
        raise_error_at_node_id(binding.decl, move(error_message));
      }

      current_binding = static_cast<ValueBinding *>(&shadowed_binding);
    }
    function_type->signatures.reverse();

    binding.type = function_type;

    current_binding = &binding;
    size_t signature_index = function_type->signatures.size();
    while (true) {
      const auto &decl_node = m_module_obj.ast.get_node(current_binding->decl)
                                  .as_FunctionDeclNode();
      if (!decl_node.body.has_value()) {
        raise_error_at_node_id(
            current_binding->decl, "not implemented (function declaration without body)"
        );
      }
      current_binding->value = analyze_function_body(
          function_type->signatures[signature_index - 1], decl_node.body.value()
      );
      --signature_index;
      if (signature_index > 0) {
        auto &
            shadowed_binding = *m_module_obj.scope
                                    ->active_bindings[current_binding->shadowed_binding_id.value()];
        current_binding = static_cast<ValueBinding *>(&shadowed_binding);
      } else {
        break;
      }
    }
  }

  Flex<Expression> analyze_function_body(
      FunctionType::Signature &signature, NodeId function_body_node_id
  ) {
    Option<FunctionType::Signature *> old_signature = m_current_function_signature;
    m_current_function_signature = &signature;
    for (const auto &param : signature.parameters) {
      auto binding = emplace_flex<ValueBinding>();
      binding->name = param.name;
      binding->type = remove_const(param.type);
      push_binding(move(binding));
    }

    const auto &function_body_node = m_module_obj.ast.get_node(function_body_node_id)
                                         .as_FunctionBodyNode();

    if (function_body_node.is_default || function_body_node.is_deleted) {
      raise_error_at_node_id(
          function_body_node_id, "not implemented (defaulted or deleted function)"
      );
    }

    Flex<Expression> result;
    if (function_body_node.expr.has_value()) {
      auto expr = build_expression(function_body_node.expr.value());
      if (is_unknown_type(signature.return_type)) {
        signature.return_type = remove_const(expr->type);
        result = expr;
      } else {
        result = require_coerce(
            signature.return_type,
            expr,
            "Cannot convert expression of type '{1}' to expected return type '{2}'"
        );
      }
    } else {
      result = build_expr_seq(function_body_node_id, function_body_node.stmts.value().data());

      if (is_unknown_type(signature.return_type)) {
        // function had no declared return type, and also contained no return statements
        signature.return_type = NULL_TYPE;
      } else if (!is_never_type(result->type) && !is_null_type(signature.return_type)) {
        // function body does not return a value on all code paths, and we can't default to null
        raise_error_at_node_id(
            function_body_node_id, "Non-null function does not return a value on all code paths"
        );
      }
    }

    for (size_t i = 0; i < signature.parameters.size(); ++i) {
      pop_binding();
    }
    m_current_function_signature = old_signature;

    return result;
  }

  FunctionType::Signature analyze_function_signature(NodeId signature_node_id) {
    const auto &signature_node = m_module_obj.ast.get_node(signature_node_id)
                                     .as_FunctionSignatureNode();
    FunctionType::Signature result;
    Set<Text> seen_param_names;
    for (NodeId parameter_node_id : signature_node.parameters) {
      auto param = analyze_function_parameter(parameter_node_id);
      if (seen_param_names.has(param.name)) {
        String error_message = "Duplicate parameter name '";
        error_message.append(param.name);
        error_message.append("' in function signature");
        raise_error_at_node_id(parameter_node_id, move(error_message));
      }
      seen_param_names.add(param.name);
      result.parameters.push_back(move(param));
    }
    if (signature_node.return_type.has_value()) {
      result.return_type = evaluate_type_expr(signature_node.return_type.value());
    } else {
      result.return_type = UNKNOWN_TYPE;
    }
    return result;
  }

  FunctionType::Parameter analyze_function_parameter(NodeId parameter_node_id) {
    const auto &parameter_node = m_module_obj.ast.get_node(parameter_node_id)
                                     .as_FunctionParameterNode();
    const auto &parameter_name_node = m_module_obj.ast.get_node(parameter_node.name);
    if (parameter_name_node.type() != NodeType::IdentifierNode) {
      raise_error_at_node_id(parameter_node.name, "not implemented (function param not Ident)");
    }

    FunctionType::Parameter result;
    result.name = parameter_name_node.as_IdentifierNode().name;
    if (!parameter_node.type.has_value()) {
      raise_error_at_node_id(
          parameter_node.type.value(), "not implemented (missing function param type annotation)"
      );
    }
    result.type = evaluate_type_expr(parameter_node.type.value());
    if (parameter_node.default_value.has_value()) {
      result.default_value = expect_expression_of_type(
          result.type, parameter_node.default_value.value()
      );
    }
    return result;
  }

  Flex<Expression> expect_expression_of_type(Flex<Type> expected_type, NodeId expr_node_id) {
    auto expr = build_expression(expr_node_id);
    return require_coerce(expected_type, expr);
  }

  Flex<Expression> build_expression(NodeId expr_node_id) {
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id);
    Flex<Expression> result;
    switch (expr_node.type()) {
    case NodeType::NumberLiteralNode:
      result = build_expr_number_literal(expr_node_id);
      break;
    case NodeType::IdentifierNode:
      result = build_expr_identifier(expr_node_id);
      break;
    case NodeType::BooleanLiteralNode:
      result = build_expr_boolean_literal(expr_node_id);
      break;
    case NodeType::BuiltinTypeNode:
      result = build_expr_builtin_type(expr_node_id);
      break;
    case NodeType::BlockExprNode:
      result = build_expr_block(expr_node_id);
      break;
    case NodeType::ExprStmtNode:
      result = build_expr_expression_statement(expr_node_id);
      break;
    case NodeType::EmptyStmtNode:
      result = Flex<EmptyExpression>::emplace();
      result->node_id = expr_node_id;
      result->type = NULL_TYPE;
      break;
    case NodeType::LetDeclNode:
    case NodeType::ConstDeclNode:
      result = build_expr_value_binding(expr_node_id, ConstSlice<NodeId>());
      break;
    case NodeType::TypeDeclNode:
      result = build_expr_type_binding(expr_node_id, ConstSlice<NodeId>());
      break;
    case NodeType::ReturnStmtNode:
      result = build_expr_return(expr_node_id);
      break;
    case NodeType::FunctionCallExprNode:
      result = build_expr_function_call(expr_node_id);
      break;
    case NodeType::RefExprNode:
      result = build_expr_ref(expr_node_id);
      break;
    case NodeType::ParenthesizedExprNode:
      result = build_expr_paren(expr_node_id);
      break;
    case NodeType::CharLiteralNode:
      result = build_expr_char_literal(expr_node_id);
      break;
    case NodeType::StringLiteralNode:
      result = build_expr_string_literal(expr_node_id);
      break;
    case NodeType::BracketExprNode:
      result = build_expr_bracket(expr_node_id);
      break;
    case NodeType::NegateExprNode:
    case NodeType::PositiveExprNode:
    case NodeType::NotExprNode:
    case NodeType::BitwiseNotExprNode:
    case NodeType::PreDecrementStmtNode:
    case NodeType::PostDecrementStmtNode:
    case NodeType::PreIncrementStmtNode:
    case NodeType::PostIncrementStmtNode:
      result = build_expr_unary_op(expr_node_id);
      break;
    case NodeType::AddExprNode:
    case NodeType::SubtractExprNode:
    case NodeType::MultiplyExprNode:
    case NodeType::DivideExprNode:
    case NodeType::AndExprNode:
    case NodeType::BitwiseAndExprNode:
    case NodeType::BitwiseOrExprNode:
    case NodeType::BitwiseXorExprNode:
    case NodeType::EqualsExprNode:
    case NodeType::GreaterExprNode:
    case NodeType::GreaterEqualsExprNode:
    case NodeType::LessExprNode:
    case NodeType::LessEqualsExprNode:
    case NodeType::LeftShiftExprNode:
    case NodeType::ModuloExprNode:
    case NodeType::NotEqualsExprNode:
    case NodeType::OrExprNode:
    case NodeType::RightShiftExprNode:
    case NodeType::AddAssignStmtNode:
    case NodeType::AssignmentStmtNode:
    case NodeType::BitwiseAndAssignStmtNode:
    case NodeType::BitwiseOrAssignStmtNode:
    case NodeType::BitwiseXorAssignStmtNode:
    case NodeType::DivAssignStmtNode:
    case NodeType::LeftShiftAssignStmtNode:
    case NodeType::ModAssignStmtNode:
    case NodeType::MulAssignStmtNode:
    case NodeType::RightShiftAssignStmtNode:
    case NodeType::SubAssignStmtNode:
      result = build_expr_binary_op(expr_node_id);
      break;
    case NodeType::AsExprNode:
      result = build_expr_as(expr_node_id);
      break;
    default:
      raise_error_at_node_id(
          expr_node_id, "not implemented (unknown node type in build_expression)"
      );
    }

    return result;
  }

  Flex<Expression> build_expr_as(NodeId expr_node_id) {
    const auto &as_node = m_module_obj.ast.get_node(expr_node_id).as_AsExprNode();
    auto expr = build_expression(as_node.expr);
    auto target_type = evaluate_type_expr(as_node.type);
    auto result = cast(target_type, expr->type, expr);

    if (!result.has_value()) {
      String error_message = "Cannot cast expression of type '";
      expr->type->serialize().to_string(error_message);
      error_message.append("' to type '");
      target_type->serialize().to_string(error_message);
      error_message.append("'");
      raise_error_at_node_id(expr_node_id, move(error_message));
    }

    // in case the type coerced without actually changing
    result.value()->type = target_type;

    return result.value();
  }

  Flex<Type> read_expr_list(List<Flex<Expression>> &output, ConstSlice<NodeId> expr_node_ids) {
    // TODO: ellipsis

    for (NodeId sub_expr_node_id : expr_node_ids) {
      output.push_back(build_expression(sub_expr_node_id));
    }

    Flex<Type> result_type = NEVER_TYPE;
    for (size_t i = 0; i < output.size(); ++i) {
      Flex<Expression> &elem = output[i];
      if (is_never_type(result_type)) {
        result_type = remove_const(elem->type);
      } else {
        auto elem_type = remove_const(elem->type);
        if (
            // types are different
            !unify(result_type, elem_type) &&
            // both are integer types
            is_integral_type(elem_type) &&
            is_integral_type(result_type)
            // the type of this elem can represent the entire range of the current inferred type
            && can_type_represent_range(
                   *elem_type, min_value_of_type(result_type), max_value_of_type(result_type)
               )
            // and the bit size of this elem type is larger
            && repr_bit_size(elem_type) > repr_bit_size(result_type)
        ) {
          result_type = elem_type;
        }

        if (
            // types are different
            !unify(result_type, elem_type) &&
            // inferred type is float and elem type is double
            is_float_type(result_type) && is_double_type(elem_type)
        ) {
          result_type = elem_type;
        }
      }
    }

    // coerce each expression to the inferred type
    for (size_t i = 0; i < output.size(); ++i) {
      Flex<Expression> &elem = output[i];
      elem = require_coerce(result_type, move(elem));
    }

    return result_type;
  }

  Flex<Expression> build_expr_bracket(NodeId expr_node_id) {
    const auto &bracket_node = m_module_obj.ast.get_node(expr_node_id).as_BracketExprNode();
    auto result = emplace_flex<ArrayLiteralExpression>();
    result->node_id = expr_node_id;
    auto array_type = emplace_flex<ArrayType>();
    array_type->element_type = read_expr_list(result->elements, bracket_node.exprs.data());
    array_type->size = result->elements.size();
    result->type = array_type;
    return result;
  }

  Flex<Expression> build_expr_char_literal(NodeId expr_node_id) {
    const auto &char_node = m_module_obj.ast.get_node(expr_node_id).as_CharLiteralNode();
    auto result = emplace_flex<CharLiteralExpression>();
    result->node_id = expr_node_id;
    auto type = emplace_flex<ConstCharacterType>();
    type->value = char_node.code_point;
    result->type = type;
    result->value = char_node.code_point;
    return result;
  }

  Flex<Expression> build_expr_string_literal(NodeId expr_node_id) {
    const auto &string_node = m_module_obj.ast.get_node(expr_node_id).as_StringLiteralNode();
    auto result = emplace_flex<StringLiteralExpression>();
    result->node_id = expr_node_id;
    auto type = emplace_flex<ConstStringType>();
    type->value = string_node.contents;
    result->type = type;
    result->value = string_node.contents;
    return result;
  }

  Flex<Expression> build_expr_paren(NodeId expr_node_id) {
    const auto &paren_node = m_module_obj.ast.get_node(expr_node_id).as_ParenthesizedExprNode();
    if (paren_node.exprs.size() == 0) {
      auto result = emplace_flex<NullLiteralExpression>();
      result->node_id = expr_node_id;
      result->type = NULL_TYPE;
      return result;
    } else if (paren_node.exprs.size() == 1) {
      return build_expression(paren_node.exprs[0]);
    } else {
      return build_expr_tuple(expr_node_id, paren_node.exprs.data());
    }
  }

  Flex<Expression> build_expr_tuple(NodeId expr_node_id, ConstSlice<NodeId> expr_node_ids) {
    auto result = emplace_flex<TupleExpression>();
    result->node_id = expr_node_id;
    for (NodeId sub_expr_node_id : expr_node_ids) {
      result->elements.push_back(build_expression(sub_expr_node_id));
    }
    auto tuple_type = emplace_flex<TupleType>();
    for (const auto &element : result->elements) {
      tuple_type->element_types.push_back(element->type);
    }
    result->type = tuple_type;
    return result;
  }

  Flex<Expression> build_expr_ref(NodeId expr_node_id) {
    const auto &ref_node = m_module_obj.ast.get_node(expr_node_id).as_RefExprNode();
    auto referent_expr = build_expression(ref_node.expr);
    switch (referent_expr->kind) {
    case ExpressionKind::Identifier: {
      auto &binding = static_cast<IdentifierExpression &>(*referent_expr).binding;
      if (binding->kind == BindingKind::Variable || binding->kind == BindingKind::Constant) {
        auto reference_type = emplace_flex<ReferenceType>();
        reference_type->referent = referent_expr->type;
        reference_type->is_const = ref_node.is_const;
        if (!reference_type->is_const && binding->kind == BindingKind::Constant) {
          String error_message = "Cannot take mutable reference to constant '";
          error_message.append(binding->name);
          error_message.append("'");
          raise_error_at_node_id(expr_node_id, move(error_message));
        }
        reference_type->is_move = ref_node.is_move;
        auto result = emplace_flex<AddressOfExpression>();
        result->node_id = expr_node_id;
        result->type = reference_type;
        result->operand = referent_expr;
        return result;
      } else {
        raise_error_at_node_id(
            expr_node_id, "not implemented (ref of non-variable/constant identifier)"
        );
      }
    }
    default:
      raise_error_at_node_id(expr_node_id, "not implemented (ref of non-identifier expression)");
    }
  }

  Flex<Expression> build_expr_function_call(NodeId expr_node_id) {
    const auto &call_node = m_module_obj.ast.get_node(expr_node_id).as_FunctionCallExprNode();
    auto callee_expr = build_expression(call_node.callee);
    List<Flex<Expression>> pos_args;
    Map<Text, Flex<Expression>> named_args;
    for (const auto &arg_node_id : call_node.args) {
      const auto &arg_node = m_module_obj.ast.get_node(arg_node_id).as_FunctionArgumentNode();
      auto expr = build_expression(arg_node.expr);
      if (arg_node.name.has_value()) {
        const auto &name_node = m_module_obj.ast.get_node(arg_node.name.value())
                                    .as_IdentifierNode();
        if (named_args.has(name_node.name)) {
          String error_message = "Duplicate argument name '";
          error_message.append(name_node.name);
          error_message.append("' in function call");
          raise_error_at_node_id(arg_node.name.value(), move(error_message));
        }
        named_args.set(name_node.name, move(expr));
      } else {
        if (named_args.size() > 0) {
          raise_error_at_node_id(
              arg_node_id, "Positional arguments must appear before named arguments"
          );
        }
        pos_args.push_back(expr);
      }
    }
    auto result = resolve_function_call(expr_node_id, callee_expr, pos_args.data(), named_args);
    if (!result.has_value()) {
      String error_message = "No function for call to '";
      error_message.append(static_cast<const FunctionType &>(*callee_expr->type).name);
      error_message.append("' matches the given arguments (");
      for (size_t i = 0; i < pos_args.size(); ++i) {
        if (i > 0) {
          error_message.append(", ");
        }
        pos_args[i]->type->serialize().to_string(error_message);
      }
      size_t named_arg_count = 0;
      for (const auto &named_arg : named_args) {
        if (named_arg_count > 0 || pos_args.size() > 0) {
          error_message.append(", ");
        }
        error_message.append(named_arg.first);
        error_message.append(": ");
        named_arg.second->type->serialize().to_string(error_message);
        ++named_arg_count;
      }
      error_message.append(")");
      raise_error_at_node_id(expr_node_id, move(error_message));
    }
    return result.value();
  }

  Option<Flex<FunctionCallExpression>> resolve_function_call(
      NodeId expr_node_id,
      Flex<Expression> callee,
      Slice<Flex<Expression>> pos_args,
      const Map<Text, Flex<Expression>> &named_args
  ) {
    if (callee->type->kind != TypeKind::Function) {
      raise_error_at_node_id(expr_node_id, "not implemented (called expression is not a function)");
    }

    // First, we try to find a signature that exactly matches the types we passed. Then, we jump
    // back to start and look for the first signature (in source declaration order) that is
    // callable via implicit conversion of the passed args.
    // TODO: do this in a single pass
    bool exact_match_only = true;
  start:

    List<Option<Flex<Expression>>> arguments;
    for (FunctionType::Signature &signature :
         static_cast<FunctionType &>(*callee->type).signatures) {
      arguments.clear();

      size_t pos_arg_index = 0;
      size_t used_named_args = 0;
      for (FunctionType::Parameter &param : signature.parameters) {
        if (pos_arg_index < pos_args.size()) {
          Option<Flex<Expression>> expr;
          if (exact_match_only) {
            expr = unify(param.type, pos_args[pos_arg_index]->type) ? pos_args[pos_arg_index]
                                                                    : Option<Flex<Expression>>();
          } else {
            expr = coerce(param.type, pos_args[pos_arg_index]);
          }
          if (!expr.has_value()) {
            goto fail;
          }
          arguments.push_back(expr);
          ++pos_arg_index;
        } else if (named_args.has(param.name)) {
          Option<Flex<Expression>> expr;
          if (exact_match_only) {
            auto arg_expr = named_args[param.name];
            expr = unify(param.type, arg_expr->type) ? arg_expr : Option<Flex<Expression>>();
          } else {
            auto arg_expr = named_args[param.name];
            expr = coerce(param.type, arg_expr);
          }
          if (!expr.has_value()) {
            goto fail;
          }
          arguments.push_back(expr);
          ++used_named_args;
        } else if (param.default_value.has_value()) {
          arguments.push_back(None());
        } else {
          goto fail;
        }
      }

      if (pos_arg_index < pos_args.size() || used_named_args < named_args.size()) {
        goto fail;
      }

      {
        auto result = emplace_flex<FunctionCallExpression>();
        result->node_id = expr_node_id;
        result->type = signature.return_type.weak();
        result->callee = callee;
        result->arguments = move(arguments);
        result->signature = &signature;
        return result;
      }

    fail:;
    }

    if (exact_match_only) {
      exact_match_only = false;
      goto start;
    }

    return None();
  }

  Flex<Expression> build_expr_return(NodeId expr_node_id) {
    const auto &return_node = m_module_obj.ast.get_node(expr_node_id).as_ReturnStmtNode();
    auto result = emplace_flex<ReturnExpression>();
    result->node_id = expr_node_id;
    if (return_node.expr.has_value()) {
      result->value = assign_current_function_return_value(build_expression(return_node.expr.value()
      ));
    } else {
      Flex<Expression> implied_return_value = Flex<NullLiteralExpression>::emplace();
      implied_return_value->node_id = expr_node_id;
      implied_return_value->type = NULL_TYPE;
      result->value = assign_current_function_return_value(implied_return_value);
    }
    result->type = NEVER_TYPE;
    return result;
  }

  Flex<Expression> assign_current_function_return_value(Flex<Expression> return_value) {
    if (!m_current_function_signature.has_value()) {
      raise_error_at_node_id(return_value->node_id, "Return value not within function");
    }

    if (is_unknown_type(m_current_function_signature.value()->return_type)) {
      m_current_function_signature.value()->return_type = remove_const(return_value->type);
      return return_value;
    }

    return require_coerce(
        m_current_function_signature.value()->return_type,
        return_value,
        "Cannot convert expression of type '{1}' to expected return type '{2}'"
    );
  }

  Flex<Expression> build_expr_expression_statement(NodeId expr_node_id) {
    const auto &expr_stmt_node = m_module_obj.ast.get_node(expr_node_id).as_ExprStmtNode();
    return build_expression(expr_stmt_node.expr);
  }

  Flex<Expression> build_expr_block(NodeId expr_node_id) {
    const BlockExprNode &block_expr_node = m_module_obj.ast.get_node(expr_node_id)
                                               .as_BlockExprNode();
    return build_expr_seq(expr_node_id, block_expr_node.stmts.data());
  }

  Flex<Expression> build_expr_seq(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    auto result = Flex<SequenceExpression>::emplace();
    result->type = NULL_TYPE;
    result->node_id = expr_node_id;
    for (size_t expr_index = 0; expr_index < stmts.size(); ++expr_index) {
      const auto &expr_node = m_module_obj.ast.get_node(stmts[expr_index]);
      if (is_value_binding_node_type(expr_node.type())) {
        auto expr = build_expr_value_binding(
            stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
        );
        result->type = expr->type;
        result->exprs.push_back(expr);
        break;
      } else if (is_type_binding_node_type(expr_node.type())) {
        auto expr = build_expr_type_binding(
            stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
        );
        result->type = expr->type;
        result->exprs.push_back(expr);
        break;
      }

      auto expr = build_expression(stmts[expr_index]);
      result->exprs.push_back(expr);
      if (expr_index == stmts.size() - 1 && !is_never_type(result->type)) {
        result->type = expr->type;
      }
    }
    return result;
  }

  Flex<Expression> build_expr_type_binding(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    if (node.type() == NodeType::TypeDeclNode) {
      return build_expr_type_decl(expr_node_id, stmts);
    } else {
      raise_error_at_node_id(
          expr_node_id, "not implemented (unknown node type in build_expr_type_binding)"
      );
    }
  }

  Flex<Expression> build_expr_type_decl(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const auto &type_decl_node = m_module_obj.ast.get_node(expr_node_id).as_TypeDeclNode();
    const auto &type_name_node = m_module_obj.ast.get_node(type_decl_node.name).as_IdentifierNode();

    auto binding = emplace_flex<TypeBinding>();
    binding->decl = expr_node_id;
    binding->name = type_name_node.name;
    binding->kind = BindingKind::Type;
    binding->visibility = DeclarationVisibility::Default;
    push_binding(binding);

    analyze_binding(binding);

    auto result = emplace_flex<TypeBindingExpression>();
    result->name = binding->name;
    result->binding = binding;
    result->body = build_expr_seq(expr_node_id, stmts);
    result->type = result->body->type;

    pop_binding();

    return result;
  }

  Flex<Expression> build_expr_value_binding(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    if (node.type() == NodeType::LetDeclNode || node.type() == NodeType::ConstDeclNode) {
      return build_expr_var_decl(expr_node_id, stmts);
    } else if (node.type() == NodeType::FunctionDeclNode) {
      return build_expr_fun_decl(expr_node_id, stmts);
    } else {
      raise_error_at_node_id(
          expr_node_id, "not implemented (unknown node type in build_expr_value_binding)"
      );
    }
  }

  Flex<Expression> build_expr_fun_decl(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const auto &fun_decl_node = m_module_obj.ast.get_node(expr_node_id).as_FunctionDeclNode();
    const auto &fun_name_node = m_module_obj.ast.get_node(fun_decl_node.name).as_IdentifierNode();

    size_t prior_bindings_size = m_module_obj.scope->active_bindings.size();

    auto binding = emplace_flex<ValueBinding>();
    binding->decl = expr_node_id;
    binding->name = fun_name_node.name;
    binding->kind = BindingKind::Function;
    binding->visibility = DeclarationVisibility::Default;
    push_binding(binding);

    // Swallow any subsequent function declarations with the same name into the same overload set
    while (true) {
      if (stmts.size() == 0) {
        break;
      }

      const auto &next_stmt_node = m_module_obj.ast.get_node(stmts[0]);
      if (next_stmt_node.type() != NodeType::FunctionDeclNode) {
        break;
      }

      const auto &next_fun_decl_node = next_stmt_node.as_FunctionDeclNode();
      const auto &next_fun_name_node = m_module_obj.ast.get_node(next_fun_decl_node.name)
                                           .as_IdentifierNode();
      if (next_fun_name_node.name != binding->name) {
        break;
      }

      auto next_fun_binding = emplace_flex<ValueBinding>();
      next_fun_binding->decl = stmts[0];
      next_fun_binding->name = next_fun_name_node.name;
      next_fun_binding->kind = BindingKind::Function;
      next_fun_binding->visibility = DeclarationVisibility::Default;
      push_binding(next_fun_binding);
      binding = next_fun_binding;
      stmts = ConstSlice<NodeId>(stmts.ptr() + 1, stmts.size() - 1);
    }

    analyze_binding(binding);

    auto result = emplace_flex<ValueBindingExpression>();
    result->binding = binding;
    result->body = build_expr_seq(expr_node_id, stmts);
    result->type = result->body->type;

    while (m_module_obj.scope->active_bindings.size() > prior_bindings_size) {
      pop_binding();
    }

    return result;
  }

  Flex<Expression> build_expr_var_decl(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    NodeId target;
    Option<NodeId> type;
    Option<NodeId> expr;
    bool is_const;
    if (node.type() == NodeType::LetDeclNode) {
      const LetDeclNode &let_decl_node = node.as_LetDeclNode();
      target = let_decl_node.target;
      type = let_decl_node.type;
      expr = let_decl_node.expr;
      is_const = false;
    } else {
      const ConstDeclNode &const_decl_node = node.as_ConstDeclNode();
      target = const_decl_node.target;
      type = const_decl_node.type;
      expr = const_decl_node.expr;
      is_const = true;
    }
    auto binding = emplace_flex<ValueBinding>();
    binding->decl = expr_node_id;
    binding->name = m_module_obj.ast.get_node(target).as_IdentifierNode().name;
    binding->kind = is_const ? BindingKind::Constant : BindingKind::Variable;
    binding->visibility = DeclarationVisibility::Default;

    if (type.has_value()) {
      binding->type = evaluate_type_expr(type.value());
      if (expr.has_value()) {
        binding->value = expect_expression_of_type(binding->type.value(), expr.value());
      }
    } else {
      binding->type = UNKNOWN_TYPE;
      if (expr.has_value()) {
        binding->value = build_expression(expr.value());
        binding->type = is_const ? binding->value.value()->type
                                 : remove_const(binding->value.value()->type);
      }
    }

    auto result = emplace_flex<ValueBindingExpression>();
    result->binding = binding;
    push_binding(move(binding));
    result->body = build_expr_seq(expr_node_id, stmts);
    result->type = result->body->type;
    pop_binding();
    return result;
  }

  Flex<Expression> build_expr_builtin_type(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const BuiltinTypeNode &expr_node = node.as_BuiltinTypeNode();
    switch (expr_node.kind) {
    case BuiltinKind::Null: {
      auto result = emplace_flex<NullLiteralExpression>();
      result->node_id = expr_node_id;
      result->type = NULL_TYPE;
      return result;
    }
    default:
      raise_error_at_node_id(expr_node_id, "not implemented");
    }
  }

  Flex<Expression> build_expr_boolean_literal(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const BooleanLiteralNode &expr_node = node.as_BooleanLiteralNode();
    auto result = emplace_flex<BooleanLiteralExpression>();
    result->node_id = expr_node_id;
    result->value = expr_node.value;
    result->type = make_flex(ConstBooleanType(expr_node.value));
    return result;
  }

  Flex<Expression> build_expr_unary_op(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    NodeId operand_node_id;
    UnaryOperatorKind op_kind;
    switch (node.type()) {
    case NodeType::NegateExprNode: {
      const NegateExprNode &negate_node = node.as_NegateExprNode();
      operand_node_id = negate_node.expr;
      op_kind = UnaryOperatorKind::Negate;
      break;
    }
    case NodeType::PositiveExprNode: {
      const PositiveExprNode &positive_node = node.as_PositiveExprNode();
      operand_node_id = positive_node.expr;
      op_kind = UnaryOperatorKind::Positive;
      break;
    }
    case NodeType::NotExprNode: {
      const NotExprNode &not_node = node.as_NotExprNode();
      operand_node_id = not_node.expr;
      op_kind = UnaryOperatorKind::Not;
      break;
    }
    case NodeType::BitwiseNotExprNode: {
      const BitwiseNotExprNode &bitwise_not_node = node.as_BitwiseNotExprNode();
      operand_node_id = bitwise_not_node.expr;
      op_kind = UnaryOperatorKind::BitwiseNot;
      break;
    }
    case NodeType::PreDecrementStmtNode: {
      const PreDecrementStmtNode &pre_dec_node = node.as_PreDecrementStmtNode();
      operand_node_id = pre_dec_node.target;
      op_kind = UnaryOperatorKind::Decrement;
      break;
    }
    case NodeType::PostDecrementStmtNode: {
      const PostDecrementStmtNode &post_dec_node = node.as_PostDecrementStmtNode();
      operand_node_id = post_dec_node.target;
      op_kind = UnaryOperatorKind::Decrement;
      break;
    }
    case NodeType::PreIncrementStmtNode: {
      const PreIncrementStmtNode &pre_inc_node = node.as_PreIncrementStmtNode();
      operand_node_id = pre_inc_node.target;
      op_kind = UnaryOperatorKind::Increment;
      break;
    }
    case NodeType::PostIncrementStmtNode: {
      const PostIncrementStmtNode &post_inc_node = node.as_PostIncrementStmtNode();
      operand_node_id = post_inc_node.target;
      op_kind = UnaryOperatorKind::Increment;
      break;
    }
    default:
      raise_error_at_node_id(
          expr_node_id, "not implemented (unknown unary op in build_expr_unary_op)"
      );
    }

    auto operand_expr = build_expression(operand_node_id);
    auto result = perform_unary_op(expr_node_id, op_kind, operand_expr->type, operand_expr);
    if (!result.has_value()) {
      switch (op_kind) {
      case UnaryOperatorKind::Negate:
      case UnaryOperatorKind::Positive:
      case UnaryOperatorKind::BitwiseNot:
      case UnaryOperatorKind::Decrement:
      case UnaryOperatorKind::Increment:
        // TODO: attempt numeric coercion
        break;
      case UnaryOperatorKind::Not: {
        auto coerced_operand = coerce(BOOL_TYPE, operand_expr);
        if (coerced_operand.has_value()) {
          result = perform_unary_op(expr_node_id, op_kind, BOOL_TYPE, coerced_operand.value());
        }
      } break;
      }
    }

    if (!result.has_value()) {
      String error_message = "Cannot apply unary operator '";
      serialize_unary_operator_kind(op_kind).to_string(error_message);
      error_message.append("' to expression of type '");
      operand_expr->type->serialize().to_string(error_message);
      error_message.append("'");
      raise_error_at_node_id(expr_node_id, move(error_message));
    }

    return result.value();
  }

  Option<Flex<Expression>> perform_unary_op(
      NodeId operation_node_id,
      UnaryOperatorKind op_kind,
      Flex<Type> operand_type,
      Flex<Expression> operand
  ) {
    resolve_update(operand_type);

    switch (op_kind) {
    case UnaryOperatorKind::Negate:
      return perform_unary_op_negate(operation_node_id, move(operand_type), move(operand));
    case UnaryOperatorKind::Positive:
      return perform_unary_op_positive(move(operand_type), move(operand));
    case UnaryOperatorKind::Not:
      return perform_unary_op_not(operation_node_id, move(operand_type), move(operand));
    case UnaryOperatorKind::BitwiseNot:
      return perform_unary_op_bitwise_not(operation_node_id, move(operand_type), move(operand));
    case UnaryOperatorKind::Decrement:
      return perform_unary_op_decrement(operation_node_id, move(operand_type), move(operand));
    case UnaryOperatorKind::Increment:
      return perform_unary_op_increment(operation_node_id, move(operand_type), move(operand));
    }
  }

  Option<Flex<Expression>> perform_unary_op_negate(
      NodeId operation_node_id, Flex<Type> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_negate_builtin(
          operation_node_id,
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::BitInt:
      return perform_unary_op_negate_bitint(
          operation_node_id,
          operand_type.derive(static_cast<BitIntType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_unary_op_negate_const_integer(
          operation_node_id,
          operand_type.derive(static_cast<ConstIntegerType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstRational:
      return perform_unary_op_negate_const_rational(
          operation_node_id,
          operand_type.derive(static_cast<ConstRationalType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return perform_unary_op_negate_const_character(
          operation_node_id,
          operand_type.derive(static_cast<ConstCharacterType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_negate_bitint(
      NodeId operation_node_id, Flex<BitIntType>, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = operand->type;
    result->op_kind = UnaryOperatorKind::Negate;
    result->operand = operand;
    return result;
  }

  Option<Flex<Expression>> perform_unary_op_negate_builtin(
      NodeId operation_node_id, Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = operand->type;
      result->op_kind = UnaryOperatorKind::Negate;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = UnaryOperatorKind::Negate;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_unary_op_negate_const_character(
      NodeId operation_node_id, Flex<ConstCharacterType> operand_type, Flex<Expression> operand
  ) {
    return perform_unary_op_negate_const_integer(
        operation_node_id,
        Flex<ConstIntegerType>::emplace(static_cast<int64_t>(operand_type->value)),
        move(operand)
    );
  }

  Option<Flex<Expression>> perform_unary_op_negate_const_integer(
      NodeId operation_node_id, Flex<ConstIntegerType> operand_type, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = Flex<ConstIntegerType>::emplace(-operand_type->value);
    result->op_kind = UnaryOperatorKind::Negate;
    result->operand = operand;
    return result;
  }

  Option<Flex<Expression>> perform_unary_op_negate_const_rational(
      NodeId operation_node_id, Flex<ConstRationalType> operand_type, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = Flex<ConstRationalType>::emplace(-operand_type->value);
    result->op_kind = UnaryOperatorKind::Negate;
    result->operand = operand;
    return result;
  }

  Option<Flex<Expression>> perform_unary_op_bitwise_not(
      NodeId operation_node_id, Flex<Type> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_bitwise_not_builtin(
          operation_node_id,
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::BitInt:
      return perform_unary_op_bitwise_not_bitint(
          operation_node_id,
          operand_type.derive(static_cast<BitIntType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_unary_op_bitwise_not_const_integer(
          operation_node_id,
          operand_type.derive(static_cast<ConstIntegerType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstRational:
      return None();
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return perform_unary_op_bitwise_not_const_character(
          operation_node_id,
          operand_type.derive(static_cast<ConstCharacterType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_bitwise_not_bitint(
      NodeId operation_node_id, Flex<BitIntType>, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = operand->type;
    result->op_kind = UnaryOperatorKind::BitwiseNot;
    result->operand = operand;
    return result;
  }

  Option<Flex<Expression>> perform_unary_op_bitwise_not_builtin(
      NodeId operation_node_id, Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = operand->type;
      result->op_kind = UnaryOperatorKind::BitwiseNot;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = UnaryOperatorKind::BitwiseNot;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_unary_op_bitwise_not_const_character(
      NodeId operation_node_id, Flex<ConstCharacterType> operand_type, Flex<Expression> operand
  ) {
    return perform_unary_op_bitwise_not_const_integer(
        operation_node_id,
        Flex<ConstIntegerType>::emplace(Integer(operand_type->value)),
        move(operand)
    );
  }

  Option<Flex<Expression>> perform_unary_op_bitwise_not_const_integer(
      NodeId operation_node_id, Flex<ConstIntegerType> operand_type, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = Flex<ConstIntegerType>::emplace(~operand_type->value);
    result->op_kind = UnaryOperatorKind::BitwiseNot;
    result->operand = operand;
    return result;
  }

  Option<Flex<Expression>> perform_unary_op_positive(
      Flex<Type> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_positive_builtin(
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)), move(operand)
      );
    case TypeKind::BitInt:
      return operand;
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      return operand;
    case TypeKind::ConstRational:
      return operand;
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return operand;
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_positive_builtin(
      Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
      return operand;
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_unary_op_not(
      NodeId operation_node_id, Flex<Type> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_not_builtin(
          operation_node_id,
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstRational)");
    case TypeKind::ConstBoolean:
      return perform_unary_op_not_const_boolean(
          operation_node_id,
          operand_type.derive(static_cast<ConstBooleanType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_not_builtin(
      NodeId operation_node_id, Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Bool: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = operand->type;
      result->op_kind = UnaryOperatorKind::Not;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_unary_op_not_const_boolean(
      NodeId operation_node_id, Flex<ConstBooleanType> operand_type, Flex<Expression> operand
  ) {
    auto result = emplace_flex<BuiltinUnaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = Flex<ConstBooleanType>::emplace(!operand_type->value);
    result->op_kind = UnaryOperatorKind::Not;
    result->operand = operand;
    return result;
  }

  void assert_mutable_operand(Flex<Expression> operand) {
    if (operand->kind == ExpressionKind::Identifier) {
      auto &identifier_expr = static_cast<IdentifierExpression &>(*operand);
      if (identifier_expr.binding->kind == BindingKind::Constant) {
        String error_message = "Cannot modify constant '";
        error_message.append(identifier_expr.binding->name);
        error_message.append('\'');
        raise_error_at_node_id(operand->node_id, move(error_message));
      }
      if (identifier_expr.binding->kind != BindingKind::Variable) {
        String error_message = "Cannot assign to '";
        error_message.append(identifier_expr.binding->name);
        error_message.append("' because it is not a variable");
        raise_error_at_node_id(operand->node_id, move(error_message));
      }
    } else { // TODO: field access, deref
      raise_error_at_node_id(operand->node_id, "Operand of mutation must be an assignable place");
    }
  }

  Option<Flex<Expression>> perform_unary_op_increment(
      NodeId operation_node_id, Flex<Type> operand_type, Flex<Expression> operand
  ) {
    assert_mutable_operand(operand);
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_increment_builtin(
          operation_node_id,
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_increment_builtin(
      NodeId operation_node_id, Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = UnaryOperatorKind::Increment;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_unary_op_decrement(
      NodeId operation_node_id, Flex<Type> operand_type, Flex<Expression> operand
  ) {
    assert_mutable_operand(operand);
    switch (operand_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Apply)");
    case TypeKind::Builtin:
      return perform_unary_op_decrement_builtin(
          operation_node_id,
          operand_type.derive(static_cast<BuiltinType &>(*operand_type)),
          move(operand)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operand->node_id, "not implemented (unary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_unary_op_decrement_builtin(
      NodeId operation_node_id, Flex<BuiltinType> operand_type, Flex<Expression> operand
  ) {
    switch (operand_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinUnaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = UnaryOperatorKind::Decrement;
      result->operand = operand;
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Flex<Expression> build_expr_binary_op(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    NodeId left_expr_node_id;
    NodeId right_expr_node_id;
    BinaryOperatorKind op_kind;
    switch (node.type()) {
    case NodeType::AddExprNode: {
      const AddExprNode &expr_node = node.as_AddExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Add;
    } break;
    case NodeType::SubtractExprNode: {
      const SubtractExprNode &expr_node = node.as_SubtractExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Subtract;
    } break;
    case NodeType::MultiplyExprNode: {
      const MultiplyExprNode &expr_node = node.as_MultiplyExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Multiply;
    } break;
    case NodeType::DivideExprNode: {
      const DivideExprNode &expr_node = node.as_DivideExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Divide;
    } break;
    case NodeType::AndExprNode: {
      const AndExprNode &expr_node = node.as_AndExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::And;
    } break;
    case NodeType::BitwiseAndExprNode: {
      const BitwiseAndExprNode &expr_node = node.as_BitwiseAndExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::BitwiseAnd;
    } break;
    case NodeType::BitwiseOrExprNode: {
      const BitwiseOrExprNode &expr_node = node.as_BitwiseOrExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::BitwiseOr;
    } break;
    case NodeType::BitwiseXorExprNode: {
      const BitwiseXorExprNode &expr_node = node.as_BitwiseXorExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::BitwiseXor;
    } break;
    case NodeType::EqualsExprNode: {
      const EqualsExprNode &expr_node = node.as_EqualsExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Equals;
    } break;
    case NodeType::GreaterExprNode: {
      const GreaterExprNode &expr_node = node.as_GreaterExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Greater;
    } break;
    case NodeType::GreaterEqualsExprNode: {
      const GreaterEqualsExprNode &expr_node = node.as_GreaterEqualsExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::GreaterEquals;
    } break;
    case NodeType::LessExprNode: {
      const LessExprNode &expr_node = node.as_LessExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Less;
    } break;
    case NodeType::LessEqualsExprNode: {
      const LessEqualsExprNode &expr_node = node.as_LessEqualsExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::LessEquals;
    } break;
    case NodeType::LeftShiftExprNode: {
      const LeftShiftExprNode &expr_node = node.as_LeftShiftExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::LeftShift;
    } break;
    case NodeType::ModuloExprNode: {
      const ModuloExprNode &expr_node = node.as_ModuloExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Modulo;
    } break;
    case NodeType::NotEqualsExprNode: {
      const NotEqualsExprNode &expr_node = node.as_NotEqualsExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::NotEquals;
    } break;
    case NodeType::OrExprNode: {
      const OrExprNode &expr_node = node.as_OrExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::Or;
    } break;
    case NodeType::RightShiftExprNode: {
      const RightShiftExprNode &expr_node = node.as_RightShiftExprNode();
      left_expr_node_id = expr_node.left;
      right_expr_node_id = expr_node.right;
      op_kind = BinaryOperatorKind::RightShift;
    } break;
    case NodeType::AssignmentStmtNode: {
      const AssignmentStmtNode &expr_node = node.as_AssignmentStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::Assignment;
    } break;
    case NodeType::BitwiseAndAssignStmtNode: {
      const BitwiseAndAssignStmtNode &expr_node = node.as_BitwiseAndAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::BitAndAssignment;
    } break;
    case NodeType::BitwiseOrAssignStmtNode: {
      const BitwiseOrAssignStmtNode &expr_node = node.as_BitwiseOrAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::BitOrAssignment;
    } break;
    case NodeType::BitwiseXorAssignStmtNode: {
      const BitwiseXorAssignStmtNode &expr_node = node.as_BitwiseXorAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::BitXorAssignment;
    } break;
    case NodeType::DivAssignStmtNode: {
      const DivAssignStmtNode &expr_node = node.as_DivAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::DivAssignment;
    } break;
    case NodeType::LeftShiftAssignStmtNode: {
      const LeftShiftAssignStmtNode &expr_node = node.as_LeftShiftAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::LShiftAssignment;
    } break;
    case NodeType::ModAssignStmtNode: {
      const ModAssignStmtNode &expr_node = node.as_ModAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::ModAssignment;
    } break;
    case NodeType::MulAssignStmtNode: {
      const MulAssignStmtNode &expr_node = node.as_MulAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::MulAssignment;
    } break;
    case NodeType::RightShiftAssignStmtNode: {
      const RightShiftAssignStmtNode &expr_node = node.as_RightShiftAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::RShiftAssignment;
    } break;
    case NodeType::SubAssignStmtNode: {
      const SubAssignStmtNode &expr_node = node.as_SubAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::SubAssignment;
    } break;
    case NodeType::AddAssignStmtNode: {
      const AddAssignStmtNode &expr_node = node.as_AddAssignStmtNode();
      left_expr_node_id = expr_node.target;
      right_expr_node_id = expr_node.expr;
      op_kind = BinaryOperatorKind::AddAssignment;
    } break;
    default:
      raise_error_at_node_id(expr_node_id, "not implemented (unknown binary op node type)");
    }

    auto left_expr = build_expression(left_expr_node_id);
    auto original_left_type = left_expr->type;
    auto right_expr = build_expression(right_expr_node_id);
    auto original_right_type = right_expr->type;

    auto left_type = left_expr->type;
    auto right_type = right_expr->type;

    auto result = perform_binary_op(
        op_kind, expr_node_id, left_type, left_expr, right_type, right_expr
    );
    if (!result.has_value() && !is_non_promoting_binary_op(op_kind)) {
      left_expr->type = remove_const(left_expr->type);
      right_expr->type = remove_const(right_expr->type);
      left_type = left_expr->type;
      right_type = right_expr->type;

      auto try_convert_right = coerce(left_type, right_expr);
      if (try_convert_right.has_value()) {
        result = perform_binary_op(
            op_kind, expr_node_id, left_type, left_expr, left_type, move(try_convert_right.value())
        );
      }
    }
    if (!result.has_value() && !is_non_promoting_binary_op(op_kind)) {
      auto try_convert_left = coerce(right_type, left_expr);
      if (try_convert_left.has_value()) {
        result = perform_binary_op(
            op_kind,
            expr_node_id,
            right_type,
            move(try_convert_left.value()),
            right_type,
            right_expr
        );
      }
    }
    if (!result.has_value()) {
      String error_message = "Cannot perform binary operation '";
      serialize_binary_operator_kind(op_kind).to_string(error_message);
      error_message.append("' on types '");
      original_left_type->serialize().to_string(error_message);
      error_message.append("' and '");
      original_right_type->serialize().to_string(error_message);
      error_message.append("'");
      raise_error_at_node_id(expr_node_id, move(error_message));
    }
    return result.value();
  }

  Option<Flex<Expression>> perform_binary_op(
      BinaryOperatorKind op_kind,
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    resolve_update(right_type);

    switch (op_kind) {
    case BinaryOperatorKind::Add:
      return perform_binary_op_add(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Subtract:
      return perform_binary_op_sub(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Multiply:
      return perform_binary_op_mul(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Divide:
      return perform_binary_op_div(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::And:
      return perform_binary_op_and(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitwiseAnd:
      return perform_binary_op_bitwise_and(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitwiseOr:
      return perform_binary_op_bitwise_or(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitwiseXor:
      return perform_binary_op_bitwise_xor(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Equals:
      return perform_binary_op_equals(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Greater:
      return perform_binary_op_greater(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::GreaterEquals:
      return perform_binary_op_greater_equals(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Less:
      return perform_binary_op_less(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::LessEquals:
      return perform_binary_op_less_equals(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::LeftShift:
      return perform_binary_op_left_shift(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Modulo:
      return perform_binary_op_modulo(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::NotEquals:
      return perform_binary_op_not_equals(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Or:
      return perform_binary_op_or(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::RightShift:
      return perform_binary_op_right_shift(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::Assignment:
      return perform_binary_op_assignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitAndAssignment:
      return perform_binary_op_bitandassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitOrAssignment:
      return perform_binary_op_bitorassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::BitXorAssignment:
      return perform_binary_op_bitxorassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::DivAssignment:
      return perform_binary_op_divassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::LShiftAssignment:
      return perform_binary_op_lshiftassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::ModAssignment:
      return perform_binary_op_modassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::MulAssignment:
      return perform_binary_op_mulassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::RShiftAssignment:
      return perform_binary_op_rshiftassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::SubAssignment:
      return perform_binary_op_subassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    case BinaryOperatorKind::AddAssignment:
      return perform_binary_op_addassignment(
          operation_node_id, move(left_type), move(left), move(right_type), move(right)
      );
    }
    return None();
  }

  Option<Flex<Expression>> perform_binary_op_add(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_add_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_add_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_add_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_add_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return perform_binary_op_add_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_add_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          Integer(left_type->value) + right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          Integer(left_type->value) + Integer(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          Rational(Integer(left_type->value)) + right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      String repr;
      repr.append(left_type->value);
      repr.append(right_const_string_type.value);
      result->type = Flex<ConstStringType>::emplace(move(repr));
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstStringType>::emplace(
          left_type->value + right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      String repr;
      repr.append(left_type->value);
      repr.append(right_const_character_type.value);
      result->type = Flex<ConstStringType>::emplace(move(repr));
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value + Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value + right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value + Rational(Integer(right_const_character_type.value))
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value + right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          Rational(left_type->value) + right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value + Integer(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::Add;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_add_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::Add;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_sub(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_sub_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_sub_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_sub_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_sub_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return perform_binary_op_sub_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_sub_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    return perform_binary_op_sub_const_integer(
        operation_node_id,
        Flex<ConstIntegerType>::emplace(Integer(left_type->value)),
        left,
        right_type,
        right
    );
  }

  Option<Flex<Expression>> perform_binary_op_sub_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value - Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value - right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value - Rational(Integer(right_const_character_type.value))
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_sub_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value - right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          Rational(left_type->value) - right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value - right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_sub_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::Subtract;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_sub_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::Subtract;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_mul(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_mul_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_mul_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_mul_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_mul_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      return perform_binary_op_mul_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_mul_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    return perform_binary_op_mul_const_integer(
        operation_node_id,
        Flex<ConstIntegerType>::emplace(Integer(left_type->value)),
        left,
        right_type,
        right
    );
  }

  Option<Flex<Expression>> perform_binary_op_mul_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value * Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value * right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value * Rational(Integer(right_const_character_type.value))
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_mul_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value * right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          Rational(left_type->value) * right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value * right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_mul_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::Multiply;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_mul_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::Multiply;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_div(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_div_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_div_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_div_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_div_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_div_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value / Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          left_type->value / right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_div_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value / right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstRationalType>::emplace(
          Rational(left_type->value) / right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_div_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::Divide;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_div_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::Divide;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_and(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_and_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      return perform_binary_op_and_const_boolean(
          operation_node_id,
          left_type.derive(static_cast<ConstBooleanType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_and_const_boolean(
      NodeId operation_node_id,
      Flex<ConstBooleanType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstBoolean) {
      auto &right_const_boolean_type = static_cast<ConstBooleanType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value && right_const_boolean_type.value
      );
      result->op_kind = BinaryOperatorKind::And;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_and_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Bool: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::And;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_and(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitwise_and_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_bitwise_and_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_bitwise_and_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_and_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value ^ right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::BitwiseAnd;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_and_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::BitwiseAnd;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_and_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::BitwiseAnd;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::BitwiseAnd;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_or(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitwise_or_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_bitwise_or_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_bitwise_or_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_or_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value | right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::BitwiseOr;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_or_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::BitwiseOr;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_or_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::BitwiseOr;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::BitwiseOr;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_xor(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitwise_xor_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_bitwise_xor_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_bitwise_xor_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_xor_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value ^ right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::BitwiseXor;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_xor_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::BitwiseXor;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_bitwise_xor_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::BitwiseXor;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::BitwiseXor;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Str:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_equals_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_equals_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_equals_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_equals_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return perform_binary_op_equals_const_boolean(
          operation_node_id,
          left_type.derive(static_cast<ConstBooleanType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstCharacter:
      return perform_binary_op_equals_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_equals_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) == right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_const_boolean(
      NodeId operation_node_id,
      Flex<ConstBooleanType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstBoolean) {
      auto &right_const_boolean_type = static_cast<ConstBooleanType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_boolean_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) == right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) == right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value == right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_equals_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::Equals;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_equals_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Null: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::Equals;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_greater_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_greater_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_greater_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_greater_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return None();
    case TypeKind::ConstCharacter:
      return perform_binary_op_greater_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_greater_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) > right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) > right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) > right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value > right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::Greater;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_greater_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::Greater;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_greater_equals_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_greater_equals_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_greater_equals_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_greater_equals_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return None();
    case TypeKind::ConstCharacter:
      return perform_binary_op_greater_equals_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_greater_equals_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) >= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) >= right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) >= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value >= right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::GreaterEquals;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_greater_equals_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::GreaterEquals;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_less_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_less_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_less_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_less_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return None();
    case TypeKind::ConstCharacter:
      return perform_binary_op_less_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_less_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) < right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) < right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) < right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value < right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::Less;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_less_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::Less;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_less_equals_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_less_equals_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_less_equals_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_less_equals_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return None();
    case TypeKind::ConstCharacter:
      return perform_binary_op_less_equals_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_less_equals_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) <= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) <= right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) <= right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value <= right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::LessEquals;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_less_equals_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::LessEquals;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_left_shift(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_left_shift_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_left_shift_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_left_shift_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_left_shift_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      if (right_const_integer_type.value < 0 || right_const_integer_type.value > UINT32_MAX) {
        return None();
      }
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value << right_const_integer_type.value.to_uint32()
      );
      result->op_kind = BinaryOperatorKind::LeftShift;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_left_shift_bitint(
      NodeId operation_node_id,
      Flex<BitIntType>,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!is_native_integral_type(right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::LeftShift;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_left_shift_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!is_native_integral_type(right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::LeftShift;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::LeftShift;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_modulo(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_modulo_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_modulo_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_modulo_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_modulo_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value % right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::Modulo;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_modulo_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::Modulo;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_modulo_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::Modulo;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::Modulo;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_not_equals_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_not_equals_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_not_equals_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      return perform_binary_op_not_equals_const_rational(
          operation_node_id,
          left_type.derive(static_cast<ConstRationalType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstBoolean:
      return perform_binary_op_not_equals_const_boolean(
          operation_node_id,
          left_type.derive(static_cast<ConstBooleanType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstCharacter:
      return perform_binary_op_not_equals_const_character(
          operation_node_id,
          left_type.derive(static_cast<ConstCharacterType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstString:
      return perform_binary_op_not_equals_const_string(
          operation_node_id,
          left_type.derive(static_cast<ConstStringType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) != right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_const_rational(
      NodeId operation_node_id,
      Flex<ConstRationalType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != Rational(right_const_integer_type.value)
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != Rational(right_const_character_type.value)
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_const_boolean(
      NodeId operation_node_id,
      Flex<ConstBooleanType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstBoolean) {
      auto &right_const_boolean_type = static_cast<ConstBooleanType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_boolean_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_const_character(
      NodeId operation_node_id,
      Flex<ConstCharacterType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstCharacter) {
      auto &right_const_character_type = static_cast<ConstCharacterType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_character_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Integer(left_type->value) != right_const_integer_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else if (right_type->kind == TypeKind::ConstRational) {
      auto &right_const_rational_type = static_cast<ConstRationalType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          Rational(left_type->value) != right_const_rational_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_const_string(
      NodeId operation_node_id,
      Flex<ConstStringType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstString) {
      auto &right_const_string_type = static_cast<ConstStringType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value != right_const_string_type.value
      );
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = BOOL_TYPE;
    result->op_kind = BinaryOperatorKind::NotEquals;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_not_equals_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Null: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::NotEquals;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_or(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_or_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      return perform_binary_op_or_const_boolean(
          operation_node_id,
          left_type.derive(static_cast<ConstBooleanType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_or_const_boolean(
      NodeId operation_node_id,
      Flex<ConstBooleanType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstBoolean) {
      auto &right_const_boolean_type = static_cast<ConstBooleanType &>(*right_type);
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstBooleanType>::emplace(
          left_type->value || right_const_boolean_type.value
      );
      result->op_kind = BinaryOperatorKind::Or;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_or_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!unify(left_type, right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Bool: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = BOOL_TYPE;
      result->op_kind = BinaryOperatorKind::Or;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_right_shift(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_right_shift_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_right_shift_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      return perform_binary_op_right_shift_const_integer(
          operation_node_id,
          left_type.derive(static_cast<ConstIntegerType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_right_shift_const_integer(
      NodeId operation_node_id,
      Flex<ConstIntegerType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind == TypeKind::ConstInteger) {
      auto &right_const_integer_type = static_cast<ConstIntegerType &>(*right_type);
      if (right_const_integer_type.value < 0 || right_const_integer_type.value > UINT32_MAX) {
        return None();
      }
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = Flex<ConstIntegerType>::emplace(
          left_type->value >> right_const_integer_type.value.to_uint32()
      );
      result->op_kind = BinaryOperatorKind::RightShift;
      result->left = left;
      result->right = right;
      return result;
    } else {
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_right_shift_bitint(
      NodeId operation_node_id,
      Flex<BitIntType>,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!is_native_integral_type(right_type)) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = left->type;
    result->op_kind = BinaryOperatorKind::RightShift;
    result->left = left;
    result->right = right;
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_right_shift_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (right_type->kind != TypeKind::Builtin || !is_integral_type(right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = left->type;
      result->op_kind = BinaryOperatorKind::RightShift;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = UINT_TYPE;
      result->op_kind = BinaryOperatorKind::RightShift;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_assignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_assignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_assignment_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_assignment_bitint(
      NodeId operation_node_id,
      Flex<BitIntType>,
      Flex<Expression> left,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left->type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = NULL_TYPE;
    result->op_kind = BinaryOperatorKind::Assignment;
    result->left = left;
    result->right = coerced_right_expr.value();
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_assignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::Assignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitandassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitandassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_bitandassignment_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitandassignment_bitint(
      NodeId operation_node_id,
      Flex<BitIntType>,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left->type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    }
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = NULL_TYPE;
    result->op_kind = BinaryOperatorKind::BitAndAssignment;
    result->left = left;
    result->right = coerced_right_expr.value();
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_bitandassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::BitAndAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitorassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitorassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitorassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::BitOrAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitxorassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_bitxorassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_bitxorassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::BitXorAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_divassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_divassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_divassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: 
    case BuiltinKind::Float:
    case BuiltinKind::Double: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::DivAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_lshiftassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_lshiftassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_lshiftassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!is_native_integral_type(right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::LShiftAssignment;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_modassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_modassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_modassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:  {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::ModAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_mulassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_mulassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_mulassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:  {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::MulAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_rshiftassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_rshiftassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_rshiftassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    if (!is_native_integral_type(right_type)) {
      return None();
    }
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize: {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::RShiftAssignment;
      result->left = left;
      result->right = right;
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Float:
    case BuiltinKind::Double:
    case BuiltinKind::Bool:
    case BuiltinKind::Null:
    case BuiltinKind::Never:
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_subassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_subassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right_type),
          move(right)
      );
    case TypeKind::BitInt:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of BitInt)");
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>> perform_binary_op_subassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:  {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::SubAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Option<Flex<Expression>> perform_binary_op_addassignment(
      NodeId operation_node_id,
      Flex<Type> left_type,
      Flex<Expression> left,
      Flex<Type> right_type,
      Flex<Expression> right
  ) {
    assert_mutable_operand(left);
    switch (left_type->kind) {
    case TypeKind::Alias:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Alias)");
    case TypeKind::Reference:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Reference)");
    case TypeKind::Struct:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Struct)");
    case TypeKind::Tuple:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Tuple)");
    case TypeKind::Array:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Array)");
    case TypeKind::TypeFn:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of TypeFn)");
    case TypeKind::Apply:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Apply)");
    case TypeKind::Builtin:
      return perform_binary_op_addassignment_builtin(
          operation_node_id,
          left_type.derive(static_cast<BuiltinType &>(*left_type)),
          move(left),
          move(right)
      );
    case TypeKind::BitInt:
      return perform_binary_op_addassignment_bitint(
          operation_node_id,
          left_type.derive(static_cast<BitIntType &>(*left_type)),
          move(left),
          move(right)
      );
    case TypeKind::Pointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Pointer)");
    case TypeKind::Slice:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Slice)");
    case TypeKind::Impl:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Impl)");
    case TypeKind::ConstInteger:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstInteger)");
    case TypeKind::ConstRational:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstRational)");
    case TypeKind::ConstBoolean:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstBoolean)");
    case TypeKind::ConstCharacter:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstCharacter)");
    case TypeKind::ConstString:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of ConstString)");
    case TypeKind::Class:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Class)");
    case TypeKind::Union:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Union)");
    case TypeKind::Concept:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Concept)");
    case TypeKind::Function:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Function)");
    case TypeKind::FunctionPointer:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of FunctionPointer)");
    case TypeKind::Closure:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Closure)");
    case TypeKind::Variable:
      raise_error_at_node_id(operation_node_id, "not implemented (binary op of Variable)");
    }
  }

  Option<Flex<Expression>>
  perform_binary_op_addassignment_bitint(
      NodeId operation_node_id,
      Flex<BitIntType> left_type,
      Flex<Expression> left,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    auto result = emplace_flex<BuiltinBinaryOperationExpression>();
    result->node_id = operation_node_id;
    result->type = NULL_TYPE;
    result->op_kind = BinaryOperatorKind::AddAssignment;
    result->left = left;
    result->right = coerced_right_expr.value();
    return result;
  }

  Option<Flex<Expression>> perform_binary_op_addassignment_builtin(
      NodeId operation_node_id,
      Flex<BuiltinType> left_type,
      Flex<Expression> left,
      Flex<Expression> right
  ) {
    auto coerced_right_expr = coerce(left_type, right);
    if (!coerced_right_expr.has_value()) {
      return None();
    } 
    switch (left_type->builtin_kind) {
    case BuiltinKind::Byte:
    case BuiltinKind::UByte:
    case BuiltinKind::Short:
    case BuiltinKind::UShort:
    case BuiltinKind::Int:
    case BuiltinKind::UInt:
    case BuiltinKind::Long:
    case BuiltinKind::ULong:
    case BuiltinKind::USize:
    case BuiltinKind::Float:
    case BuiltinKind::Double:  {
      auto result = emplace_flex<BuiltinBinaryOperationExpression>();
      result->node_id = operation_node_id;
      result->type = NULL_TYPE;
      result->op_kind = BinaryOperatorKind::AddAssignment;
      result->left = left;
      result->right = coerced_right_expr.value();
      return result;
    }
    case BuiltinKind::Char:
    case BuiltinKind::Bool:
    case BuiltinKind::Never:
    case BuiltinKind::Null: 
    case BuiltinKind::Str:
    case BuiltinKind::Unknown:
      return None();
    }
  }

  Flex<Expression> build_expr_identifier(NodeId node_id) {
    const auto &identifier_node = m_module_obj.ast.get_node(node_id).as_IdentifierNode();
    auto expr = Flex<IdentifierExpression>::emplace();
    auto binding = resolve_value_binding(node_id, identifier_node.name);
    expr->binding = binding.weak();
    expr->type = binding->type.value().weak();
    expr->node_id = node_id;
    return expr;
  }

  Flex<Type> evaluate_type_expr(NodeId type_expr_node_id) {
    const auto &type_expr_node = m_module_obj.ast.get_node(type_expr_node_id);
    switch (type_expr_node.type()) {
    case NodeType::IdentifierNode:
      return resolve_type_binding(type_expr_node_id, type_expr_node.as_IdentifierNode().name)
          ->type.value()
          .weak();
    case NodeType::BuiltinTypeNode:
      return evaluate_type_expr_builtin(type_expr_node.as_BuiltinTypeNode());
    case NodeType::ConstTypeExprNode:
      return evaluate_type_expr_const(type_expr_node.as_ConstTypeExprNode());
    case NodeType::RefExprNode:
      return evaluate_type_expr_ref(type_expr_node.as_RefExprNode());
    case NodeType::ParenthesizedExprNode:
      return evaluate_type_expr_paren(type_expr_node.as_ParenthesizedExprNode());
    case NodeType::IndexingExprNode:
      return evaluate_type_expr_indexing(type_expr_node.as_IndexingExprNode());
    case NodeType::DerefExprNode:
      return evaluate_type_expr_deref(type_expr_node.as_DerefExprNode());
    case NodeType::BracketExprNode:
      return evaluate_type_expr_bracket(type_expr_node.as_BracketExprNode());
    default:
      raise_error_at_node_id(type_expr_node_id, "not implemented (unknown type expr)");
    }
  }

  Flex<Type> evaluate_type_expr_bracket(const BracketExprNode &bracket_node) {
    if (bracket_node.exprs.size() == 1) {
      auto element_type = evaluate_type_expr(bracket_node.exprs[0]);
      auto result = emplace_flex<SliceType>();
      result->element_type = element_type;
      return result;
    } else if (bracket_node.exprs.size() == 2) {
      auto element_type = evaluate_type_expr(bracket_node.exprs[0]);
      auto length_expr = build_expression(bracket_node.exprs[1]);
      if (length_expr->type->kind != TypeKind::ConstInteger) {
        String
            error_message = "Expected a constant integer length, but got an expression of type '";
        length_expr->type->serialize().to_string(error_message);
        error_message.append('\'');
        raise_error_at_node_id(bracket_node.exprs[1], move(error_message));
      }
      const Integer &length_value = static_cast<const ConstIntegerType &>(*length_expr->type).value;
      if (length_value < 0 || length_value > UINT64_MAX) {
        String error_message = "Array type length must be a non-negative integer less than ";
        Integer(UINT64_MAX).to_string(error_message);
        raise_error_at_node_id(bracket_node.exprs[1], move(error_message));
      }
      auto result = emplace_flex<ArrayType>();
      result->element_type = element_type;
      result->size = length_value.to_uint32();
      return result;
    } else {
      raise_error_at_node_id(
          m_binding_currently_analyzing.value()->decl, "Invalid type expression"
      );
    }
  }

  Flex<Type> evaluate_type_expr_deref(const DerefExprNode &deref_node) {
    auto pointed_type = evaluate_type_expr(deref_node.expr);
    auto result = emplace_flex<PointerType>();
    result->pointee = pointed_type;
    result->is_const = deref_node.is_const;
    return result;
  }

  Flex<Type> evaluate_type_expr_indexing(const IndexingExprNode &indexing_node) {
    const auto &object_node = m_module_obj.ast.get_node(indexing_node.object);

    if (object_node.type() != NodeType::BitIntTypeNode) {
      raise_error_at_node_id(indexing_node.object, "not implemented (indexing of non-bitint type)");
    }

    if (indexing_node.indices.size() != 1) {
      raise_error_at_node_id(
          indexing_node.object, "not implemented (type expr indexing with multiple indices)"
      );
    }

    const auto &index_node = m_module_obj.ast.get_node(indexing_node.indices[0]).as_IndexNode();
    if (index_node.name.has_value()) {
      raise_error_at_node_id(
          indexing_node.indices[0], "not implemented (type expr indexing with named index)"
      );
    }

    auto index_expr = build_expression(index_node.value);

    if (index_expr->type->kind != TypeKind::ConstInteger) {
      String error_message = "Expected an integer constant, but got an expression of type '";
      index_expr->type->serialize().to_string(error_message);
      error_message.append('\'');
      raise_error_at_node_id(indexing_node.indices[0], move(error_message));
    }

    const Integer &index_value = static_cast<const ConstIntegerType &>(*index_expr->type).value;
    if (index_value < 1 || index_value > UINT32_MAX) {
      String error_message = "BitInt bit-count must be a positive integer less than or equal to ";
      Serialize::of(int64_t(UINT32_MAX)).to_string(error_message);
      raise_error_at_node_id(indexing_node.indices[0], move(error_message));
    }
    bool is_signed = object_node.as_BitIntTypeNode().is_signed;

    auto result = emplace_flex<BitIntType>();
    result->is_signed = is_signed;
    result->bit_width = index_value.to_uint32();
    return result;
  }

  Flex<Type> evaluate_type_expr_paren(const ParenthesizedExprNode &paren_node) {
    if (paren_node.exprs.size() == 0) {
      return NULL_TYPE;
    } else if (paren_node.exprs.size() == 1) {
      return evaluate_type_expr(paren_node.exprs[0]);
    } else {
      auto tuple_type = emplace_flex<TupleType>();
      for (NodeId sub_expr_node_id : paren_node.exprs) {
        tuple_type->element_types.push_back(evaluate_type_expr(sub_expr_node_id));
      }
      return tuple_type;
    }
  }

  Flex<Type> evaluate_type_expr_ref(const RefExprNode &ref_expr_node) {
    auto referent_type = evaluate_type_expr(ref_expr_node.expr);
    auto result = emplace_flex<ReferenceType>();
    result->referent = referent_type;
    result->is_const = ref_expr_node.is_const;
    result->is_move = ref_expr_node.is_move;
    return result;
  }

  Flex<Type> evaluate_type_expr_const(const ConstTypeExprNode &const_type_expr_node) {
    auto expr = build_expression(const_type_expr_node.expr);
    switch (expr->type->kind) {
    case TypeKind::ConstInteger:
    case TypeKind::ConstRational:
    case TypeKind::ConstBoolean:
    case TypeKind::ConstString:
    case TypeKind::ConstCharacter:
      return expr->type;
    default:
      String error_message = "Expected a constant, but got an expression of type '";
      expr->type->serialize().to_string(error_message);
      error_message.append('\'');
      raise_error_at_node_id(expr->node_id, move(error_message));
    }
  }

  Flex<Type> evaluate_type_expr_builtin(const BuiltinTypeNode &builtin_type_node) {
    switch (builtin_type_node.kind) {
    case BuiltinKind::Bool:
      return BOOL_TYPE;
    case BuiltinKind::Byte:
      return BYTE_TYPE;
    case BuiltinKind::Short:
      return SHORT_TYPE;
    case BuiltinKind::Int:
      return INT_TYPE;
    case BuiltinKind::Long:
      return LONG_TYPE;
    case BuiltinKind::UByte:
      return UBYTE_TYPE;
    case BuiltinKind::UShort:
      return USHORT_TYPE;
    case BuiltinKind::UInt:
      return UINT_TYPE;
    case BuiltinKind::ULong:
      return ULONG_TYPE;
    case BuiltinKind::USize:
      return USIZE_TYPE;
    case BuiltinKind::Float:
      return FLOAT_TYPE;
    case BuiltinKind::Double:
      return DOUBLE_TYPE;
    case BuiltinKind::Char:
      return CHAR_TYPE;
    case BuiltinKind::Str:
      return STR_TYPE;
    case BuiltinKind::Null:
      return NULL_TYPE;
    case BuiltinKind::Never:
      return NEVER_TYPE;
    default:
      throw RuntimeError("unreachable");
    }
  }

  Flex<Expression> build_expr_number_literal(NodeId expr_node_id) {
    auto expr = Flex<NumberLiteralExpression>::emplace();
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id).as_NumberLiteralNode();
    expr->node_id = expr_node_id;
    expr->value = expr_node.value;
    if (expr_node.value.has_decimal_point || expr_node.value.exponent_sign == "-") {
      String num_str;
      num_str.append(expr_node.value.base_prefix);
      num_str.append(expr_node.value.integer_digits);
      num_str.append(".");
      num_str.append(expr_node.value.fractional_digits);
      Rational result(num_str);
      if (expr_node.value.exponent_digits.size() > 0) {
        uint8_t exponent_base = 10;
        if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
          exponent_base = 2;
        }
        auto factor = Integer(exponent_base)
                          .pow(Integer(expr_node.value.exponent_digits).to_uint32());
        if (expr_node.value.exponent_sign == "-") {
          result = Rational(result.numerator(), result.denominator() * factor);
        } else {
          result = Rational(result.numerator() * factor, result.denominator());
        }
      }
      expr->type = make_flex(ConstRationalType(move(result)));
    } else {
      String num_str;
      num_str.append(expr_node.value.base_prefix);
      num_str.append(expr_node.value.integer_digits);
      Integer result(num_str);
      if (expr_node.value.exponent_digits.size() > 0) {
        uint8_t exponent_base = 10;
        if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
          exponent_base = 2;
        }
        auto factor = Integer(exponent_base)
                          .pow(Integer(expr_node.value.exponent_digits).to_uint32());
        result *= factor;
      }
      expr->type = make_flex(ConstIntegerType(move(result)));
    }
    return expr;
  }

  void collect_top_level_bindings() {
    const ModuleNode &module_node = m_module_obj.ast.get_node(m_module_obj.ast_root)
                                        .as_ModuleNode();
    for (NodeId decl_node_id : module_node.decls) {
      Binding current_binding_details{};
      current_binding_details.visibility = DeclarationVisibility::Default;
      get_binding_details(current_binding_details, decl_node_id);
      const Option<BindingId> existing_binding_id = m_module_obj.scope->active_binding_ids.find(
          current_binding_details.name
      );
      auto binding = Flex<Binding>::emplace();
      switch (current_binding_details.kind) {
      case BindingKind::Variable:
      case BindingKind::Constant:
      case BindingKind::Function:
        binding = Flex<ValueBinding>::emplace();
        break;
      case BindingKind::Type:
      case BindingKind::Class:
      case BindingKind::Concept:
        binding = Flex<TypeBinding>::emplace();
        break;
      case BindingKind::Module:
        binding = Flex<ModuleBinding>::emplace();
        break;
      }
      binding->decl = decl_node_id;
      binding->kind = current_binding_details.kind;
      binding->visibility = current_binding_details.visibility;
      binding->shadowed_binding_id = existing_binding_id;
      binding->name = move(current_binding_details.name);
      push_binding(move(binding));
    }
  }

  void get_binding_details(Binding &current_binding_details, NodeId decl_node_id) {
    const Node &decl_node = m_sema_result.modules[0].ast.get_node(decl_node_id);
    switch (decl_node.type()) {
    case NodeType::LetDeclNode: {
      const auto &n = decl_node.as_LetDeclNode();
      current_binding_details.kind = BindingKind::Variable;
      get_binding_details(current_binding_details, n.target);
      break;
    }
    case NodeType::ConstDeclNode: {
      const auto &n = decl_node.as_ConstDeclNode();
      current_binding_details.kind = BindingKind::Constant;
      get_binding_details(current_binding_details, n.target);
      break;
    }
    case NodeType::IdentifierNode: {
      const auto &n = decl_node.as_IdentifierNode();
      current_binding_details.name = n.name;
      break;
    }
    case NodeType::FunctionDeclNode: {
      const FunctionDeclNode &n = decl_node.as_FunctionDeclNode();
      current_binding_details.kind = BindingKind::Function;
      get_binding_details(current_binding_details, n.name);
      break;
    }
    case NodeType::TypeDeclNode: {
      const TypeDeclNode &n = decl_node.as_TypeDeclNode();
      current_binding_details.kind = BindingKind::Type;
      get_binding_details(current_binding_details, n.name);
      break;
    }
    default:
      raise_error_at_node_id(decl_node_id, "not implemented (unknown top-level decl node)");
    }
  }

private:
  [[noreturn]] void raise_error_at_node_id(NodeId node_id, String &&error_message) {
    const Node &node = m_module_obj.ast.get_node(node_id);
    raise_error_at_node(node, move(error_message));
  }

  [[noreturn]] void raise_error_at_node(const Node &node, String &&error_message) {
    const Token &token = m_module_obj.tokens.get_token(node.start_token());
    throw SourceLocationError(token.location, move(error_message));
  }

  SemaResult &m_sema_result;
  Module &m_module_obj;
  Option<FunctionType::Signature *> m_current_function_signature;
  Option<Binding *> m_binding_currently_analyzing;
};

class SemaState {
public:
  SemaState(SemaResult &sema_result) : m_sema_result(sema_result) {}

  void analyze() {
    set_up_dep_counts();
    Option<ModuleId> module_id;
    while ((module_id = select_module_to_analyze()).has_value()) {
      SemaWorkerState(m_sema_result, m_sema_result.modules[module_id.value()]).analyze_module();
    }
  }

private:
  void set_up_dep_counts() {
    while (m_module_dep_counts.size() < m_sema_result.modules.size()) {
      m_module_dep_counts.push_back(0);
    }
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      m_module_dep_counts[i] = m_sema_result.modules[i].imported_ids.size();
    }
  }

  Option<ModuleId> select_module_to_analyze() {
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      Module &module_obj = m_sema_result.modules[i];
      if (module_obj.analyzed) {
        continue;
      }
      if (m_module_dep_counts[i] == 0) {
        if (module_obj.group_module_ids.size() != 0) {
          // For cooperation with other threads, we should only ever analyze the "head" of a
          // module group - that is, the module in the group with the lowest ID.
          for (ModuleId group_module_id : module_obj.group_module_ids) {
            if (group_module_id < static_cast<ModuleId>(i)) {
              goto cont;
            }
          }
        }
        return Some(i);
      }
    cont:;
    }
    return None();
  }

  SemaResult &m_sema_result;
  List<uint32_t> m_module_dep_counts;
};

} // namespace

void Analyzer::analyze(SemaResult &sema_result) {
  SemaState(sema_result).analyze();
}

} // namespace amelia
