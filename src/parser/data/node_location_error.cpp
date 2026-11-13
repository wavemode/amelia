#include "node_location_error.hpp"

#include "util/data/string.hpp"

namespace amelia {

NodeLocationError::NodeLocationError(NodeId node_loc, const String &message)
    : RuntimeError(message.c_str()), node_id(node_loc) {}

} // namespace amelia
