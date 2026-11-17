#include "node_location_error.hpp"

#include "util/data/string.hpp"

namespace amelia {

NodeLocationError::NodeLocationError(NodeId node_loc, String &&message)
    : RuntimeError(move(message)), node_id(node_loc) {}

} // namespace amelia
