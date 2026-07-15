#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/utility.hpp"

namespace amelia {

class Serialize;
using NodeId = int32_t;

struct Expression : FlexFromThis<Expression>, WithDynamicId {
  Flex<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;

  virtual ~Expression() = default;
};

template <typename T> struct ExpressionWithDynamicId : Expression {
  ExpressionWithDynamicId() {
    m_dynamic_id = type_id<T>();
  }
};

} // namespace amelia
