#pragma once

#include <cstdint>

#include "util/data/utility.hpp"

namespace amelia {

using NodeId = int32_t;
struct SourceFile;

struct NodeRef {
  NodeId node_id;
  SourceFile *source_file;

  bool operator==(const NodeRef &other) const {
    return node_id == other.node_id && source_file == other.source_file;
  }

  bool operator!=(const NodeRef &other) const {
    return !(*this == other);
  }
};

} // namespace amelia
