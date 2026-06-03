#pragma once

#include "prelude.hpp"

#include "data/source/declaration_visibility.hpp"
#include "data/sema/type_kind.hpp"

namespace amelia {

enum class PrimitiveKind {
  Byte,
  UByte,
  Short,
  UShort,
  Int,
  UInt,
  Long,
  ULong,
  Float,
  Double,
  Bool,
  Char,
  Str,
  Null,
  Never,
  Unknown,
};

class Type {
public:
  struct Alias {};
  struct Apply {};
  struct Primitive {
    PrimitiveKind primitive_kind;
  };
  struct Bitint {};
  struct Tuple {};
  struct Struct {};
  struct Reference {};
  struct Pointer {};
  struct Array {};
  struct Slice {};
  struct Impl {};
  struct Const {};
  struct Class {};
  struct Union {};
  struct Function {};
  struct FunctionPointer {};
  struct Concept {};
  struct Variable {};
  struct TypeDecl {};
  struct ModuleDecl {};

#define X(TYPE_KIND)                                                                               \
  Type(DeclVisibility vis, TYPE_KIND type)                                                  \
      : visibility(vis), m_kind(TypeKind::TYPE_KIND), m_data(move(type)) {}                   \
                                                                                                   \
  TYPE_KIND &as_##TYPE_KIND() {                                                                    \
    if (m_kind != TypeKind::TYPE_KIND) {                                                           \
      throw RuntimeError("Type kind mismatch");                                              \
    }                                                                                              \
    return m_data.data_##TYPE_KIND;                                                                \
  }                                                                                                \
                                                                                                   \
  const TYPE_KIND &as_##TYPE_KIND() const {                                                        \
    if (m_kind != TypeKind::TYPE_KIND) {                                                           \
      throw RuntimeError("Type kind mismatch");                                              \
    }                                                                                              \
    return m_data.data_##TYPE_KIND;                                                                \
  }
  TYPE_KIND_LIST
#undef X

  ~Type() {
    m_data.destroy(m_kind);
  }

  Type(const Type &other)
      : visibility(other.visibility), m_kind(other.m_kind), m_data(other.m_kind, other.m_data),
        memo_name(other.memo_name) {}

  Type(Type &&other) noexcept
      : visibility(other.visibility), m_kind(other.m_kind),
        m_data(other.m_kind, move(other.m_data)), memo_name(move(other.memo_name)) {}

  TypeKind kind() const {
    return m_kind;
  }

  Type &operator=(const Type &other) {
    if (this != &other) {
      visibility = other.visibility;
      memo_name = other.memo_name;
      if (m_kind == other.m_kind) {
        m_data.assign(m_kind, other.m_data);
      } else {
        m_data.destroy(m_kind);
        m_kind = other.m_kind;
        new (&m_data) TypeData(m_kind, other.m_data);
      }
    }
    return *this;
  }

  Type &operator=(Type &&other) {
    if (this != &other) {
      visibility = other.visibility;
      memo_name = move(other.memo_name);
      if (m_kind == other.m_kind) {
        m_data.assign(m_kind, move(other.m_data));
      } else {
        m_data.destroy(m_kind);
        m_kind = other.m_kind;
        new (&m_data) TypeData(m_kind, move(other.m_data));
      }
    }
    return *this;
  }

  DeclVisibility visibility;
  Option<String> memo_name;

private:
  union TypeData {
#define X(TYPE_KIND)                                                                               \
  TYPE_KIND data_##TYPE_KIND;                                                                      \
                                                                                                   \
  explicit TypeData(TYPE_KIND type) : data_##TYPE_KIND(move(type)) {}
    TYPE_KIND_LIST
#undef X

    TypeData(TypeKind kind, const TypeData &other) {
      switch (kind) {
#define X(TYPE_KIND)                                                                               \
  case TypeKind::TYPE_KIND:                                                                        \
    new (&data_##TYPE_KIND) TYPE_KIND(other.data_##TYPE_KIND);                                     \
    break;
        TYPE_KIND_LIST
#undef X
      }
    }

    TypeData(TypeKind kind, TypeData &&other) {
      switch (kind) {
#define X(TYPE_KIND)                                                                               \
  case TypeKind::TYPE_KIND:                                                                        \
    new (&data_##TYPE_KIND) TYPE_KIND(move(other.data_##TYPE_KIND));                          \
    break;
        TYPE_KIND_LIST
#undef X
      }
    }

    void assign(TypeKind kind, const TypeData &other) {
      switch (kind) {
#define X(TYPE_KIND)                                                                               \
  case TypeKind::TYPE_KIND:                                                                        \
    data_##TYPE_KIND = other.data_##TYPE_KIND;                                                     \
    break;
        TYPE_KIND_LIST
#undef X
      }
    }

    void assign(TypeKind kind, TypeData &&other) {
      switch (kind) {
#define X(TYPE_KIND)                                                                               \
  case TypeKind::TYPE_KIND:                                                                        \
    data_##TYPE_KIND = move(other.data_##TYPE_KIND);                                          \
    break;
        TYPE_KIND_LIST
#undef X
      }
    }

    void destroy(TypeKind kind) noexcept {
      switch (kind) {
#define X(TYPE_KIND)                                                                               \
  case TypeKind::TYPE_KIND:                                                                        \
    data_##TYPE_KIND.~TYPE_KIND();                                                                 \
    break;
        TYPE_KIND_LIST
#undef X
      }
    }

    ~TypeData() {}
  };

  TypeKind m_kind;
  TypeData m_data;
};

} // namespace amelia
