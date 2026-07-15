#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/utility.hpp"

namespace amelia {

class Serialize;
using NodeId = int32_t;

struct Expression : FlexFromThis<Expression>, Dynamic {
  Flex<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;

  virtual ~Expression() = default;
};

} // namespace amelia
