#pragma once

#include <cstdint>

#include "util/data/runtime_error.hpp"

namespace amelia {

class String;
using NodeId = int32_t;

/**
 * @class NodeLocationError
 * @brief Exception associated with a specific node in the AST.
 */
struct NodeLocationError : public RuntimeError {
  NodeLocationError(NodeId node_loc, const String &message);

  const NodeId node_id;
};

} // namespace amelia
