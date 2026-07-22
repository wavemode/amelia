#include "function_call_expression.hpp"

#include "util/data/integer.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize FunctionCallExpression::serialize() const {
  Serialize result;
  result.set_object_name("FunctionCallExpression");
  result.add_object_field("callee", callee->serialize());
  result.add_object_field("signature_id", Integer(signature_id).serialize());
  if (arguments.size() > 0) {
    Serialize args_ser;
    for (const auto &arg : arguments) {
      if (arg.has_value()) {
        args_ser.add_list_item(arg.value()->serialize());
      } else {
        args_ser.add_list_item(Serialize::literal("(default)"));
      }
    }
    result.add_object_field("arguments", move(args_ser));
  }
  if (implicit_arguments.size() > 0) {
    Serialize implicit_args_ser;
    for (const auto &[key, value] : implicit_arguments) {
      if (value.has_value()) {
        implicit_args_ser.add_object_field(String(key), value.value()->serialize());
      } else {
        implicit_args_ser.add_object_field(String(key), Serialize::literal("(default)"));
      }
    }
    result.add_object_field("implicit_arguments", move(implicit_args_ser));
  }
  return result;
}

} // namespace amelia
