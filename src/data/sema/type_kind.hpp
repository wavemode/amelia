#define TYPE_KIND_LIST                                                                             \
  X(Alias)                                                                                         \
  X(Apply)                                                                                         \
  X(Builtin)                                                                                       \
  X(Bitint)                                                                                        \
  X(Tuple)                                                                                         \
  X(Struct)                                                                                        \
  X(Reference)                                                                                     \
  X(Pointer)                                                                                       \
  X(Array)                                                                                         \
  X(Slice)                                                                                         \
  X(Impl)                                                                                          \
  X(TypeExpr)                                                                                      \
  X(Const)                                                                                         \
  X(Class)                                                                                         \
  X(Union)                                                                                         \
  X(Function)                                                                                      \
  X(FunctionPointer)                                                                               \
  X(Concept)                                                                                       \
  X(Variable)

namespace amelia {

enum class TypeKind {
#define X(TYPE_KIND) TYPE_KIND,
  TYPE_KIND_LIST
#undef X
};

} // namespace amelia
