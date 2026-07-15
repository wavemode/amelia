#include "statement_sequence.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize StatementSequence::serialize() const {
  Serialize result;
  result.set_object_name("StatementSequence");
  Serialize stmts_ser;
  if (stmts.size() > 0) {
    for (const auto &expr : stmts) {
      stmts_ser.add_list_item(expr->serialize());
    }
    result.add_object_field("stmts", move(stmts_ser));
  }
  return result;
}

} // namespace amelia
