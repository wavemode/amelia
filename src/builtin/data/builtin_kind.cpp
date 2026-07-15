#include "builtin_kind.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_builtin_kind(BuiltinKind kind) {
  String result;
  switch (kind) {
  case BuiltinKind::Byte:
    result.append("byte");
    break;
  case BuiltinKind::UByte:
    result.append("ubyte");
    break;
  case BuiltinKind::Short:
    result.append("short");
    break;
  case BuiltinKind::UShort:
    result.append("ushort");
    break;
  case BuiltinKind::Int:
    result.append("int");
    break;
  case BuiltinKind::UInt:
    result.append("uint");
    break;
  case BuiltinKind::Long:
    result.append("long");
    break;
  case BuiltinKind::ULong:
    result.append("ulong");
    break;
  case BuiltinKind::USize:
    result.append("usize");
    break;
  case BuiltinKind::Float:
    result.append("float");
    break;
  case BuiltinKind::Double:
    result.append("double");
    break;
  case BuiltinKind::Bool:
    result.append("bool");
    break;
  case BuiltinKind::Char:
    result.append("char");
    break;
  case BuiltinKind::Str:
    result.append("str");
    break;
  case BuiltinKind::Null:
    result.append("null");
    break;
  case BuiltinKind::Never:
    result.append("never");
    break;
  case BuiltinKind::Unknown:
    result.append("unknown");
    break;
  }
  return Serialize::literal(move(result));
} // namespace amelia

} // namespace amelia